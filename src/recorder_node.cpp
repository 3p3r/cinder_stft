#include "recorder_node.h"

namespace cieq {
namespace audio {

RecorderNode::RecorderNode(AudioRecorder& rec)
	: cinder::audio::NodeAutoPullable(RecorderNode::Format())
	, mAudioRecorder(rec)
{}

void RecorderNode::process(cinder::audio::Buffer*)
{
	mAudioRecorder.update();
}

}
} //!cieq::audio