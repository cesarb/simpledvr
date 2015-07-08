#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QGst/Init>
#include <QGst/Quick/VideoSurface>

#include <QGst/Bin>
#include <QGst/Element>
#include <QGst/ElementFactory>
#include <QGst/GhostPad>

#include "pipeline.h"
#include "storage.h"
#include "timer.h"

static QGst::ElementPtr createTestSource()
{
    auto source = QGst::Bin::create("capture");

    auto videoSource = QGst::ElementFactory::make("videotestsrc");
    auto audioSource = QGst::ElementFactory::make("audiotestsrc");

    source->add(videoSource, audioSource);

    source->addPad(QGst::GhostPad::create(videoSource->getStaticPad("src"), "video"));
    source->addPad(QGst::GhostPad::create(audioSource->getStaticPad("src"), "audio"));

    return source;
}

static QGst::ElementPtr createCaptureSource()
{
    auto source = QGst::Bin::create("capture");

    auto videoSource = QGst::ElementFactory::make("v4l2src");
    videoSource->setProperty("device", "/dev/video0");
    videoSource->setProperty("norm", 256);

    auto deinterlace = QGst::ElementFactory::make("deinterlace");

    auto audioSource = QGst::ElementFactory::make("alsasrc");
    audioSource->setProperty("device", "hw:1,0");

    source->add(videoSource, deinterlace, audioSource);

    videoSource->link(deinterlace);

    source->addPad(QGst::GhostPad::create(deinterlace->getStaticPad("src"), "video"));
    source->addPad(QGst::GhostPad::create(audioSource->getStaticPad("src"), "audio"));

    return source;
}

static QGst::ElementPtr createSoftwareEncoder()
{
    auto encoder = QGst::Bin::create("encoder");

    auto videoEncoder = QGst::ElementFactory::make("vp8enc");
    auto audioEncoder = QGst::ElementFactory::make("vorbisenc");

    encoder->add(videoEncoder, audioEncoder);

    encoder->addPad(QGst::GhostPad::create(videoEncoder->getStaticPad("sink"), "video_sink"));
    encoder->addPad(QGst::GhostPad::create(audioEncoder->getStaticPad("sink"), "audio_sink"));

    encoder->addPad(QGst::GhostPad::create(videoEncoder->getStaticPad("src"), "video_src"));
    encoder->addPad(QGst::GhostPad::create(audioEncoder->getStaticPad("src"), "audio_src"));

    return encoder;
}

static QGst::ElementPtr createVAAPIEncoder()
{
    auto encoder = QGst::Bin::create("encoder");

    auto vaapipostproc = QGst::ElementFactory::make("vaapipostproc");
    auto vaapiqueue = QGst::ElementFactory::make("queue", "vaapiQueue");
    auto vaapiencode = QGst::ElementFactory::make("vaapiencode_h264");

    auto audioEncoder = QGst::ElementFactory::make("avenc_ac3");

    encoder->add(vaapipostproc, vaapiqueue, vaapiencode, audioEncoder);

    encoder->addPad(QGst::GhostPad::create(vaapipostproc->getStaticPad("sink"), "video_sink"));
    encoder->addPad(QGst::GhostPad::create(audioEncoder->getStaticPad("sink"), "audio_sink"));

    vaapipostproc->link(vaapiqueue);
    vaapiqueue->link(vaapiencode);

    encoder->addPad(QGst::GhostPad::create(vaapiencode->getStaticPad("src"), "video_src"));
    encoder->addPad(QGst::GhostPad::create(audioEncoder->getStaticPad("src"), "audio_src"));

    return encoder;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGst::init(&argc, &argv);

    QCommandLineParser parser;
    QCommandLineOption testSourceOption("test-source");
    QCommandLineOption testEncoderOption("test-encoder");
    parser.addOptions({testSourceOption, testEncoderOption});
    parser.process(app);

    auto videoSurface = new QGst::Quick::VideoSurface;

    auto source = parser.isSet(testSourceOption) ? createTestSource() : createCaptureSource();
    auto encoderFactory = parser.isSet(testEncoderOption) ? createSoftwareEncoder : createVAAPIEncoder;

    Pipeline pipeline(source, videoSurface->videoSink(), encoderFactory);
    StorageMonitor storageMonitor;
    RecordingTimer recordingTimer;

    QObject::connect(&recordingTimer, &RecordingTimer::stopRecording, &pipeline, &Pipeline::stopRecording);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("videoSurface"), videoSurface);
    engine.rootContext()->setContextProperty(QStringLiteral("pipeline"), &pipeline);
    engine.rootContext()->setContextProperty(QStringLiteral("storageMonitor"), &storageMonitor);
    engine.rootContext()->setContextProperty(QStringLiteral("recordingTimer"), &recordingTimer);
    engine.load(QUrl(QStringLiteral("qrc:/player.qml")));

    pipeline.start();
    return app.exec();
}
