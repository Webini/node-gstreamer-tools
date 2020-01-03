#include <nan.h>

#include <gst/gst.h>
#include "Discover.h"

void DiscoverInit(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() < 3) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (!info[1]->IsNumber()) {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }
  
  Nan::Utf8String filepath(info[0]);
  double timeout = Nan::To<unsigned int>(info[1]).FromJust();
  Nan::Callback* callback = new Nan::Callback(Nan::To<v8::Function>(info[2]).ToLocalChecked());

  Nan::AsyncQueueWorker(new Discover(callback, timeout, *filepath));
}

void init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE exports) {
  v8::Local<v8::Context> context = exports->CreationContext();
  gst_init(NULL, NULL);

  exports->Set(context,
               Nan::New("discover").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(DiscoverInit)
                   ->GetFunction(context)
                   .ToLocalChecked());
}

NODE_MODULE(gst_discover, init);
