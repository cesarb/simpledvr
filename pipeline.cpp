#include "pipeline.h"

#include <QDebug>

#include <QGlib/Connect>
#include <QGst/Bin>
#include <QGst/Bus>
#include <QGst/ElementFactory>
#include <QGst/Event>
#include <QGst/GhostPad>
#include <QGst/Message>
#include <QGst/Pad>

#include <gst/gst.h>

Pipeline::Pipeline(const QGst::ElementPtr &source, const QGst::ElementPtr &videoSink, QGst::ElementPtr (*encoderElementFactory)(), QObject *parent)
    : QObject(parent), pipeline(QGst::Pipeline::create("pipeline")),
      videoRecordValve(QGst::ElementFactory::make("valve", "videoRecordValve")),
      audioRecordValve(QGst::ElementFactory::make("valve", "audioRecordValve")),
      encoderElementFactory(encoderElementFactory)
{
    pipeline->setProperty("message-forward", true);
    pipeline->bus()->addSignalWatch();
    QGlib::connect(pipeline->bus(), "message", this, &Pipeline::onBusMessage);

    auto videoTee = QGst::ElementFactory::make("tee", "videoTee");
    auto audioTee = QGst::ElementFactory::make("tee", "audioTee");

    auto videoQueue = QGst::ElementFactory::make("queue", "videoPlaybackQueue");
    auto audioQueue = QGst::ElementFactory::make("queue", "audioPlaybackQueue");

    auto audioSink = QGst::ElementFactory::make("autoaudiosink");

    pipeline->add(source, videoTee, audioTee, videoQueue, audioQueue, videoSink, audioSink);

    source->link("video", videoTee);
    source->link("audio", audioTee);

    videoTee->getRequestPad("src_%u")->link(videoQueue->getStaticPad("sink"));
    audioTee->getRequestPad("src_%u")->link(audioQueue->getStaticPad("sink"));

    videoQueue->link(videoSink);
    audioQueue->link(audioSink);

    pipeline->add(videoRecordValve, audioRecordValve);

    videoRecordValve->setProperty("drop", true);
    audioRecordValve->setProperty("drop", true);

    videoTee->getRequestPad("src_%u")->link(videoRecordValve->getStaticPad("sink"));
    audioTee->getRequestPad("src_%u")->link(audioRecordValve->getStaticPad("sink"));
}

void Pipeline::start()
{
    pipeline->setState(QGst::StatePlaying);
}

void Pipeline::stop()
{
    pipeline->setState(QGst::StateNull);
}

QGst::ElementPtr Pipeline::createRecordBin(QGst::ElementPtr (*encoderElementFactory)())
{
    auto recordBin = QGst::Bin::create("record");

    auto videoInputQueue = QGst::ElementFactory::make("queue", "videoRecordQueue");
    auto audioInputQueue = QGst::ElementFactory::make("queue", "audioRecordQueue");
    auto audioconvert = QGst::ElementFactory::make("audioconvert");

    auto encoder = encoderElementFactory();

    auto videoOutputQueue = QGst::ElementFactory::make("queue", "videoMuxQueue");
    auto audioOutputQueue = QGst::ElementFactory::make("queue", "audioMuxQueue");

    auto mux = QGst::ElementFactory::make("matroskamux", "mux");
    auto filesink = QGst::ElementFactory::make("filesink");
    filesink->setProperty("location", "test.mkv");

    recordBin->add(videoInputQueue, audioInputQueue, audioconvert, encoder, videoOutputQueue, audioOutputQueue, mux, filesink);

    recordBin->addPad(QGst::GhostPad::create(videoInputQueue->getStaticPad("sink"), "video"));
    recordBin->addPad(QGst::GhostPad::create(audioInputQueue->getStaticPad("sink"), "audio"));

    audioInputQueue->link(audioconvert);

    videoInputQueue->getStaticPad("src")->link(encoder->getStaticPad("video_sink"));
    audioconvert->getStaticPad("src")->link(encoder->getStaticPad("audio_sink"));

    encoder->getStaticPad("video_src")->link(videoOutputQueue->getStaticPad("sink"));
    encoder->getStaticPad("audio_src")->link(audioOutputQueue->getStaticPad("sink"));

    videoOutputQueue->getStaticPad("src")->link(mux->getRequestPad("video_%u"));
    audioOutputQueue->getStaticPad("src")->link(mux->getRequestPad("audio_%u"));

    mux->link(filesink);

    return recordBin;
}

void Pipeline::startRecording()
{
    if (recordBin)
        return;

    recordBin = createRecordBin(encoderElementFactory);

    pipeline->add(recordBin);

    videoRecordValve->getStaticPad("src")->link(recordBin->getStaticPad("video"));
    audioRecordValve->getStaticPad("src")->link(recordBin->getStaticPad("audio"));

    emit recordingStarting();

    recordBin->syncStateWithParent();

    videoRecordValve->setProperty("drop", false);
    audioRecordValve->setProperty("drop", false);

    emit recordingStarted();
}

bool Pipeline::sendEosEvent(const QGst::PadPtr &pad)
{
    // pad->sendEvent(QGst::EosEvent::create()) is broken
    // see https://bugzilla.gnome.org/show_bug.cgi?id=740319

    auto event = QGst::EosEvent::create();
    gst_event_ref(event);
    return gst_pad_send_event(pad, event);
}

void Pipeline::stopRecording()
{
    if (!recordBin)
        return;

    emit recordingStopping();

    videoRecordValve->setProperty("drop", true);
    audioRecordValve->setProperty("drop", true);

    sendEosEvent(recordBin->getStaticPad("video"));
    sendEosEvent(recordBin->getStaticPad("audio"));
}

void Pipeline::onEosMessage()
{
    recordBin->setState(QGst::StateNull);

    pipeline->remove(recordBin);
    recordBin.clear();

    emit recordingStopped();
}

void Pipeline::onBusMessage(const QGst::MessagePtr &message)
{
    switch (message->type()) {
    case QGst::MessageError:
        stop();
        qWarning() << "Error:" << message.staticCast<QGst::ErrorMessage>()->error().message();
        break;
    case QGst::MessageWarning:
        qWarning() << "Warning:" << message.staticCast<QGst::WarningMessage>()->error().message();
        break;
    case QGst::MessageInfo:
        qDebug() << "Info:" << message.staticCast<QGst::InfoMessage>()->error().message();
        break;
    case QGst::MessageElement:
        if (message->internalStructure()->name() == QLatin1String("GstBinForwarded")) {
            auto forwardedMessage = message->internalStructure()->value("message").get<QGst::MessagePtr>();
            if (forwardedMessage->type() == QGst::MessageEos)
                onEosMessage();
        }
        break;
    default:
        break;
    }
}
