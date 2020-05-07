#ifndef MSC_TEST_MEDIA_STREAM_TRACK_FACTORY_HPP
#define MSC_TEST_MEDIA_STREAM_TRACK_FACTORY_HPP

#include "api/create_peerconnection_factory.h"
#include "api/media_stream_interface.h"

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> createFactory();

rtc::scoped_refptr<webrtc::AudioTrackInterface> createAudioTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, const std::string& label);

rtc::scoped_refptr<webrtc::VideoTrackInterface> createVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, const std::string& label);

rtc::scoped_refptr<webrtc::VideoTrackInterface> createSquaresVideoTrack(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, const std::string& label);


#endif
