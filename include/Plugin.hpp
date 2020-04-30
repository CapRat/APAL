#ifndef PLUGIN_HPP
#define PLUGIN_HPP

class Plugin {
public:
    virtual ~Plugin() = default;
    virtual void processAudio(float**) = 0;
};

extern Plugin* createPlugin();
extern void deletePlugin();
#endif //! PLUGIN_HPP
