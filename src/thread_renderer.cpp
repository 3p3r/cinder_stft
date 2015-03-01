#include "thread_renderer.h"

namespace cieq {

ThreadRenderer::ThreadRenderer(AudioNodes& nodes)
{
	//TODO: used audio nodes to resize internal pools
}

void ThreadRenderer::update()
{

}

void ThreadRenderer::draw()
{

}

SpectralSurface& ThreadRenderer::getSurface(int index)
{
	return mSurfacePool[index];
}

} //!cieq