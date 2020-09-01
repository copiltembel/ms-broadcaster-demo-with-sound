#ifndef BROADCASTER_H
#define BROADCASTER_H

#include "mediasoupclient.hpp"
#include "json.hpp"
#include <future>
#include <string>

class Broadcaster : public mediasoupclient::SendTransport::Listener,
                    mediasoupclient::Producer::Listener
{
	/* Virtual methods inherited from SendTransport::Listener. */
public:
	std::future<void> OnConnect(
	  mediasoupclient::Transport* transport, const nlohmann::json& transportLocalParameters);
	void OnConnectionStateChange(mediasoupclient::Transport* transport, const std::string& connectionState);
	std::future<std::string> OnProduce(
	  mediasoupclient::SendTransport* /*transport*/,
	  const std::string& kind,
	  nlohmann::json rtpParameters,
	  const nlohmann::json& appData);

	/* Virtual methods inherited from Producer::Listener. */
public:
	void OnTransportClose(mediasoupclient::Producer* producer);

public:
	void Start(
	  const std::string& baseUrl,
	  bool enableAudio,
	  bool useSimulcast,
	  const nlohmann::json& routerRtpCapabilities);
	void Stop();

	std::future<std::string> OnProduceData(mediasoupclient::SendTransport* sendTransport, 
		const nlohmann::json& sctpStreamParameters, 
		const std::string& label,
		const std::string& protocol,
		const nlohmann::json& appData)
	{ 
			std::promise<std::string> promise; 
			promise.set_value(""); 
			return promise.get_future(); 
	};

private:
	mediasoupclient::Device device;

	std::string id = std::to_string(rtc::CreateRandomId());
	std::string transportId;
	std::string baseUrl;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
	
};

#endif // STOKER_HPP
