#include "recorder_node.h"

namespace cieq {
namespace audio {

RecorderNode::RecorderNode(const Format& fmt)
	: cinder::audio::NodeAutoPullable(fmt)
	, mAudioRecorder(fmt)
{}

void RecorderNode::process(cinder::audio::Buffer*)
{
	mAudioRecorder.update();
}

}
} //!cieq::audio