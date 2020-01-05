#ifndef __GLibHelpers_h__
#define __GLibHelpers_h__

#include <nan.h>
#include <gst/gst.h>

#define NAN_AUTO_SET(object, key, value) object->Set( \
  Nan::New(key).ToLocalChecked(), \
  Nan::New(value).ToLocalChecked() \
)

#define NAN_KEY_SET(object, key, value) object->Set( \
  Nan::New(key).ToLocalChecked(), \
  value \
)

using namespace v8;

Local<Object> createBuffer(char *data, int length);

Local<Value> gstsample_to_v8( GstSample *sample );
Local<Value> gstvaluearray_to_v8( const GValue *gv );
Local<Value> gvalue_to_v8( const GValue *gv );
void v8_to_gvalue( Local<Value> v, GValue *gv, GParamSpec *spec);
Local<Value> chararray_to_v8(const char *str);

void gst_tags_to_v8_iterate(const GstTagList *tags, const gchar *tag, gpointer data);
gboolean gst_structure_to_v8_value_iterate(GQuark field_id, const GValue *value, gpointer data);
Local<Object> gst_structure_to_v8( Local<Object> obj, const GstStructure *struc );


#endif