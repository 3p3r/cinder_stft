#ifndef CIEQ_INCLUDE_AUDIO_NODES_H_
#define CIEQ_INCLUDE_AUDIO_NODES_H_

#include <memory>

namespace cinder {
namespace audio {
class InputDeviceNode;
class MonitorNode;
class MonitorSpectralNode;
}} //!ci::audio

namespace cieq
{

class AppGlobals;

class AudioNodes
{
public:
	AudioNodes(AppGlobals&);

	void												setup(bool auto_enable = true);
	void												enableInput();
	void												disableInput();

	cinder::audio::InputDeviceNode* const				getInputDeviceNode();
	cinder::audio::MonitorNode* const					getMonitorNode();
	cinder::audio::MonitorSpectralNode* const			getMonitorSpectralNode();

private:
	std::shared_ptr<cinder::audio::InputDeviceNode>		mInputDeviceNode;
	std::shared_ptr<cinder::audio::MonitorNode>			mMonitorNode;
	std::shared_ptr<cinder::audio::MonitorSpectralNode>	mMonitorSpectralNode;

private:
	AppGlobals&											mGlobals;
};

} //!cieq

#endif //!CIEQ_INCLUDE_AUDIO_NODES_H_