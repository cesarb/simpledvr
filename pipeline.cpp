#include "pipeline.h"

#include <QDebug>

#include <QGlib/Connect>
#include <QGst/Bus>
#include <QGst/ElementFactory>
#include <QGst/GhostPad>
#include <QGst/Message>
#include <QGst/Pad>

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

QGst::PipelinePtr Pipeline::createRecordPipeline(QGst::ElementPtr (*encoderElementFactory)())
{
    auto recordPipeline = QGst::Pipeline::create("record");

    auto videoInputQueue = QGst::ElementFactory::make("queue", "videoRecordQueue");
    auto audioInputQueue = QGst::ElementFactory::make("queue", "audioRecordQueue");
    auto audioconvert = QGst::ElementFactory::make("audioconvert");

    auto encoder = encoderElementFactory();

    auto videoOutputQueue = QGst::ElementFactory::make("queue", "videoMuxQueue");
    auto audioOutputQueue = QGst::ElementFactory::make("queue", "audioMuxQueue");

    auto mux = QGst::ElementFactory::make("matroskamux", "mux");
    auto filesink = QGst::ElementFactory::make("filesink");
    filesink->setProperty("location", "test.mkv");

    recordPipeline->add(videoInputQueue, audioInputQueue, audioconvert, encoder, videoOutputQueue, audioOutputQueue, mux, filesink);

    recordPipeline->addPad(QGst::GhostPad::create(videoInputQueue->getStaticPad("sink"), "video"));
    recordPipeline->addPad(QGst::GhostPad::create(audioInputQueue->getStaticPad("sink"), "audio"));

    audioInputQueue->link(audioconvert);

    videoInputQueue->getStaticPad("src")->link(encoder->getStaticPad("video_sink"));
    audioconvert->getStaticPad("src")->link(encoder->getStaticPad("audio_sink"));

    encoder->getStaticPad("video_src")->link(videoOutputQueue->getStaticPad("sink"));
    encoder->getStaticPad("audio_src")->link(audioOutputQueue->getStaticPad("sink"));

    videoOutputQueue->getStaticPad("src")->link(mux->getRequestPad("video_%u"));
    audioOutputQueue->getStaticPad("src")->link(mux->getRequestPad("audio_%u"));

    mux->link(filesink);

    return recordPipeline;
}

void Pipeline::startRecording()
{
    if (recordPipeline)
        return;

    recordPipeline = createRecordPipeline(encoderElementFactory);

    pipeline->add(recordPipeline);

    videoRecordValve->getStaticPad("src")->link(recordPipeline->getStaticPad("video"));
    audioRecordValve->getStaticPad("src")->link(recordPipeline->getStaticPad("audio"));

    emit recordingStarting();

    recordPipeline->syncStateWithParent();

    videoRecordValve->setProperty("drop", false);
    audioRecordValve->setProperty("drop", false);

    emit recordingStarted();
}

void Pipeline::stopRecording()
{
    emit recordingStopping();

    videoRecordValve->setProperty("drop", true);
    audioRecordValve->setProperty("drop", true);

    // Insert EOS

}

void Pipeline::recordingEos()
{
    recordPipeline->setState(QGst::StateNull);

    pipeline->remove(recordPipeline);
    recordPipeline.clear();

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
                recordingEos();
        }
        break;
    default:
        break;
    }
}
