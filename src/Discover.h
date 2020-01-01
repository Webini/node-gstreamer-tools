#ifndef __DISCOVER_H__
#define __DISCOVER_H__

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <nan.h>
#include "GLibHelpers.h"

class Discover : public Nan::AsyncWorker {
  public:
    Discover(Nan::Callback *callback, unsigned int timeout, const char *filepath); 
    ~Discover();
    void Execute();
    void HandleOKCallback();

  private:
    unsigned int timeout;
    const gchar *filepath;
    const char *error;
    GstDiscoverer *dc;
    GstDiscovererInfo *info;
    GError *gerr;

    void processDc(v8::Local<v8::Object> &output);
    void addStreamInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output);
    void addAudioInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output);
    void addVideoInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output);
    void addSubtitleInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output);
};

#endif
