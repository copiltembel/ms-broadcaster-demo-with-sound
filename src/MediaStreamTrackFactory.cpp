#define MSC_CLASS "MediaStreamTrackFactory"

#include <iostream>

#include "MediaStreamTrackFactory.hpp"
#include "MediaSoupClientErrors.hpp"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "fakegenerators/fake_audio_capture_module.h"
#include "fakegenerators/fake_periodic_video_track_source.h"
#include "fakegenerators/frame_generator_capturer_video_track_source.h"
#include "system_wrappers/include/clock.h"
#include "fakegenerators/test_audio_device.h"

using namespace mediasoupclient;

/* MediaStreamTrack holds reference to the threads of the PeerConnectionFactory.
 * Use plain pointers in order to avoid threads being destructed before tracks.
 */
static rtc::Thread* networkThread;
static rtc::Thread* signalingThread;
static rtc::Thread* workerThread;

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> createFactory()
{
	networkThread   = rtc::Thread::CreateWithSocketServer().release();
	signalingThread = rtc::Thread::Create().release();
	workerThread    = rtc::Thread::Create().release();

	networkThread->SetName("network_thread", nullptr);
	signalingThread->SetName("signaling_thread", nullptr);
	workerThread->SetName("worker_thread", nullptr);

	if (!networkThread->Start() || !signalingThread->Start() || !workerThread->Start())
	{
		MSC_THROW_INVALID_STATE_ERROR("thread start errored");
	}

	webrtc::PeerConnectionInterface::RTCConfiguration config;

	auto queueFactory = webrtc::CreateDefaultTaskQueueFactory();

	auto testAudioCaptureModule = webrtc::TestAudioDeviceModule::Create(queueFactory.get(),
			webrtc::TestAudioDeviceModule::CreatePulsedNoiseCapturer(32000, 48000, 2),
			webrtc::TestAudioDeviceModule::CreateDiscardRenderer(48000, 2));

	if (!testAudioCaptureModule)
	{
		MSC_THROW_INVALID_STATE_ERROR("audio capture module creation errored");
	} else {
		std::cout << "Created audio capture module" << std::endl;
	}

	auto factory = webrtc::CreatePeerConnectionFactory(
	  networkThread,
	  workerThread,
	  signalingThread,
	  testAudioCaptureModule,
	  webrtc::CreateBuiltinAudioEncoderFactory(),
	  webrtc::CreateBuiltinAudioDecoderFactory(),
	  webrtc::CreateBuiltinVideoEncoderFactory(),
	  webrtc::CreateBuiltinVideoDecoderFactory(),
	  nullptr /*audio_mixer*/,
	  nullptr /*audio_processing*/);

	if (!factory)
	{
		MSC_THROW_ERROR("error ocurred creating peerconnection factory");
	}

	return factory;
}

// Audio track creation.
rtc::scoped_refptr<webrtc::AudioTrackInterface> createAudioTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, const std::string& label)
{

	cricket::AudioOptions options;
	options.highpass_filter = false;
	options.auto_gain_control = false;
	options.noise_suppression = false;
	options.echo_cancellation = false;
	options.residual_echo_detector = false;
	options.experimental_agc = false;
	options.experimental_ns = false;
	options.typing_detection = false;


	rtc::scoped_refptr<webrtc::AudioSourceInterface> source = factory->CreateAudioSource(options);
	std::cout << "Audio source created -> creating track" << std::endl;

	return factory->CreateAudioTrack(label, source);
}

// Video track creation.
rtc::scoped_refptr<webrtc::VideoTrackInterface> createVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, const std::string& label)
{
	auto video_track_source = new rtc::RefCountedObject<webrtc::FakePeriodicVideoTrackSource>(
			false /* remote */);

	return factory->CreateVideoTrack(
			rtc::CreateRandomUuid(), video_track_source);
}

rtc::scoped_refptr<webrtc::VideoTrackInterface> createSquaresVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, const std::string& label)
{
	std::cout << "[INFO] getting frame generator" << std::endl;
	auto video_track_source_ = new rtc::RefCountedObject<webrtc::FrameGeneratorCapturerVideoTrackSource>(
		webrtc::FrameGeneratorCapturerVideoTrackSource::Config(), 
		webrtc::Clock::GetRealTimeClock(),
		false);
	video_track_source_->Start();

	std::cout << "[INFO] creating video track" << std::endl;
	return factory->CreateVideoTrack(
			rtc::CreateRandomUuid(), video_track_source_);
}
