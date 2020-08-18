#include <tools/PortHandling.hpp>
using namespace APAL;
size_t
APAL::getAudioChannelCount(IPlugin* plug, APAL::PortDirection dir)
{
  size_t size = 0;
  iteratePorts<IAudioPort>(plug, dir, [&size](IAudioPort* p, size_t) {
    size += p->size();
    return false;
  });
  return size;
}

void
APAL::iterateAudioChannels(
  IPlugin* plug,
  std::function<bool(IAudioPort*, IAudioChannel*, size_t)> iterFunc)
{
  size_t counter = 0;
  iteratePorts<IAudioPort>(plug, [&counter, &iterFunc](IAudioPort* p, size_t) {
    for (size_t i = 0; i < p->size(); i++) {
      if (iterFunc(p, p->at(i), counter))
        return true;
      counter++;
    }
    return false;
  });
}
IAudioChannel*
APAL::getAudioChannelFromIndex(IPlugin* plug, size_t index)
{
  APAL::IAudioChannel* channel = nullptr;
  iterateAudioChannels(
    plug,
    [&channel, index](IAudioPort*, IAudioChannel* c, size_t channelIndex) {
      if (index == channelIndex) {
        channel = c;
        return true;
      }
      return false;
    });
  return channel;
}
void
APAL::iteratePortsFlat(
  IPlugin* plug,
  std::function<bool(APAL::IPort* p, size_t ind)> iterFunc)
{
  size_t index = 0;
  for (size_t i = 0; i < plug->getPortComponent()->size(); i++) {
    auto aPort = dynamic_cast<IAudioPort*>(plug->getPortComponent()->at(i));
    if (aPort == nullptr) {
      if (iterFunc(plug->getPortComponent()->at(i), index++))
        return;
    } else {
      for (size_t j = 0; j < aPort->size(); j++)
        if (iterFunc(plug->getPortComponent()->at(i), index++))
          return;
    }
  }
}