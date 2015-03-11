#ifndef CIEQ_INCLUDE_MONITOR_RENDERER_H_
#define CIEQ_INCLUDE_MONITOR_RENDERER_H_

namespace cistft {

class AppGlobals;

class MonitorRenderer
{
public:
	MonitorRenderer(AppGlobals&);
	void			draw();

private:
	AppGlobals&		mGlobals;
};

} // !namespace cistft

#endif // !CIEQ_INCLUDE_MONITOR_RENDERER_H_