#ifndef CIEQ_INCLUDE_MONITOR_RENDERER_H_
#define CIEQ_INCLUDE_MONITOR_RENDERER_H_

namespace cieq {

class AppGlobals;

class MonitorRenderer
{
public:
	MonitorRenderer(AppGlobals&);
	void			draw();

private:
	AppGlobals&		mGlobals;
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_MONITOR_RENDERER_H_