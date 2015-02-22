#ifndef CIEQ_INCLUDE_RECORDER_NODE_H_
#define CIEQ_INCLUDE_RECORDER_NODE_H_

#include <cinder/audio/Node.h>
#include "audio_recorder.h"

namespace cieq {
namespace audio {

class RecorderNode : public ci::audio::NodeAutoPullable
{
public:
	struct Format
		: public ci::audio::Node::Format
		, public AudioRecorder::Options
	{};

	RecorderNode(AudioRecorder&);
	virtual ~RecorderNode() {}

protected:
	void process( cinder::audio::Buffer* )	override;

private:
	AudioRecorder&	mAudioRecorder;
};

}} // !namespace cieq::audio

#endif // !CIEQ_INCLUDE_RECORDER_NODE_H_