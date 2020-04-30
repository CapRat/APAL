#ifndef PLUGIN_HPP
#define PLUGIN_HPP

class Plugin {
public:
	virtual ~Plugin() = default;
	virtual processAudio(float**) = 0;
};

extern Plugin* createPlugin();
extern deletePlugin();
#endif //! PLUGIN_HPP