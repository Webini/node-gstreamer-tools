#include <nan.h>
#include <gst/gst.h>
#include "GLibHelpers.h"

unsigned long count_glist(const GList *list) {
  unsigned long len = 0;
  for (const GList *p = list; p; p = p->next) {
    len++;
  }
  return len;
}

void GetPlugins(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  GList *plugins = gst_registry_get_plugin_list(gst_registry_get());
  v8::Local<v8::Array> arr = Nan::New<v8::Array>(count_glist(plugins));

  unsigned long i = 0;
  for (GList *p = plugins; p; p = p->next, i++) {
    GstPlugin *plugin = (GstPlugin *)(p->data);
    ARRAY_SET(arr, i, Nan::New(gst_plugin_get_name(plugin)).ToLocalChecked());
  }

  gst_plugin_list_free(plugins);
  info.GetReturnValue().Set(arr);
}

unsigned long count_features(GList *firstFeature) {
  GList *features = firstFeature;

  unsigned long featuresLen = 0;
  while (features) {
    if (features->data == NULL) {
      continue;
    }

    features = g_list_next(features);
    featuresLen++;
  }

  return featuresLen;
}

void add_factory_details(GstElementFactory *factory, v8::Local<v8::Object> &output) {
  // details
  gchar **keys = gst_element_factory_get_metadata_keys(factory);
  if (keys != NULL) {
    for (gchar **k = keys; *k != NULL; ++k) {
      OBJECT_SET(output, *k, chararray_to_v8(gst_element_factory_get_metadata(factory, *k)));
    }
    g_strfreev (keys);
  }
}

v8::Local<v8::Array> add_caps(const GstCaps *caps) {
  if (caps == NULL || gst_caps_is_empty(caps)) {
    return Nan::New<v8::Array>(0);
  }

  v8::Local<v8::Array> capsList = Nan::New<v8::Array>(gst_caps_get_size(caps));
  for (unsigned long i = 0; i < gst_caps_get_size(caps); i++) {
    GstStructure *structure = gst_caps_get_structure(caps, i);
    GstCapsFeatures *features = gst_caps_get_features(caps, i);
    v8::Local<v8::Object> capsObject = Nan::New<v8::Object>();
    unsigned int featuresLen = gst_caps_features_get_size(features);
    v8::Local<v8::Array> featuresArr = Nan::New<v8::Array>(featuresLen);

    OBJECT_SET(capsObject, "features", featuresArr);
    OBJECT_SET(capsObject, "mimetype", chararray_to_v8(gst_structure_get_name(structure)));

    for (unsigned int j = 0; j < featuresLen; j++) {
      ARRAY_SET(featuresArr, j, Nan::New(gst_caps_features_get_nth(features, j)).ToLocalChecked());
    }

    gst_structure_foreach(structure, gst_structure_to_v8_value_iterate, (gpointer)&capsObject);
    ARRAY_SET(capsList, i, capsObject);
  }

  return capsList;
}

void add_pad_templates(GstPluginFeature *feature, GstElementFactory *factory, v8::Local<v8::Object> &output) {
  if (gst_element_factory_get_num_pad_templates(factory) == 0) {
    return;
  }

  const GList *pads = gst_element_factory_get_static_pad_templates(factory);
  v8::Local<v8::Array> arr = Nan::New<v8::Array>(count_glist(pads));

  unsigned long i = 0;
  for (const GList *pad = pads; pad; pad = pad->next, i++) {
    v8::Local<v8::Object> padObject = Nan::New<v8::Object>();
    GstStaticPadTemplate *padtemplate = (GstStaticPadTemplate *)(pad->data);

    OBJECT_SET(padObject, "direction", Nan::New(padtemplate->direction));
    OBJECT_SET(padObject, "presence", Nan::New(padtemplate->presence));
    OBJECT_SET(padObject, "name", chararray_to_v8(padtemplate->name_template));

    if (padtemplate->static_caps.string) {
      GstCaps *caps = gst_static_caps_get(&padtemplate->static_caps);
      if (caps != NULL) {
        OBJECT_SET(padObject, "any", Nan::New((bool)gst_caps_is_any(caps)));
        OBJECT_SET(padObject, "capabilities", add_caps(caps));
      }
      gst_caps_unref(caps);
    }

    Nan::Set(arr, i, padObject);
  }

  OBJECT_SET(output, "pads", arr);
}

void add_uri_handler_info(GstElement *element, v8::Local<v8::Object> &output) {
  if (!GST_IS_URI_HANDLER(element)) {
    return;
  }

  v8::Local<v8::Object> uriInfos = Nan::New<v8::Object>();
  const gchar *const *uri_protocols;

  OBJECT_SET(uriInfos, "type", Nan::New(gst_uri_handler_get_uri_type(GST_URI_HANDLER(element))));
  uri_protocols = gst_uri_handler_get_protocols(GST_URI_HANDLER(element));

  if (uri_protocols && *uri_protocols) {
    unsigned long uriLen = 0;
    for (const gchar *const *protocol = uri_protocols; *protocol != NULL; protocol++, uriLen++);

    v8::Local<v8::Array> arr = Nan::New<v8::Array>(uriLen);
    unsigned long i = 0;
    for (; *uri_protocols != NULL; uri_protocols++, i++) {
      ARRAY_SET(arr, i, Nan::New(*uri_protocols).ToLocalChecked());
    }
    
    OBJECT_SET(uriInfos, "protocols", arr);
  }
  
  OBJECT_SET(output, "uriHandler", uriInfos);
}

void add_preset_list(GstElement * element, v8::Local<v8::Object> &output) {
  gchar **presets;

  if (!GST_IS_PRESET(element)) {
    return;
  }

  presets = gst_preset_get_preset_names(GST_PRESET(element));
  if (presets && *presets) {
    unsigned long presetsLen = 0;
    for (gchar **preset = presets; *preset != NULL; preset++, presetsLen++);

    v8::Local<v8::Array> arr = Nan::New<v8::Array>(presetsLen);
    unsigned long i = 0;
    for (gchar **preset = presets; *preset != NULL; preset++, i++) {
      ARRAY_SET(arr, i, Nan::New(*preset).ToLocalChecked());
    }
    
    OBJECT_SET(output, "presets", arr);
    g_strfreev (presets);
  }
}

unsigned int count_hierarchy_depth(GType type, unsigned int previous = 1) {
  GType parent = g_type_parent(type);

  if (parent) {
    return count_hierarchy_depth(parent, previous + 1);
  }

  return previous;
}

unsigned int add_hierarchy(GType type, v8::Local<v8::Array> &output, unsigned int offset = 0) {
  unsigned int newOffset = offset;
  GType parent = g_type_parent(type);

  if (parent) {
    newOffset = add_hierarchy(parent, output, offset);
  }

  ARRAY_SET(output, newOffset, Nan::New(g_type_name(type)).ToLocalChecked());
  return newOffset + 1;
}

void process_element_factory(GstPluginFeature *feature, v8::Local<v8::Object> &output) {
  GstElementFactory *factory = GST_ELEMENT_FACTORY(feature);
  GstElement *element;

  if (!factory) {
    return;
  }

  element = gst_element_factory_create(factory, NULL);
  if (!element) {
    return;
  }

  OBJECT_SET(output, "name", chararray_to_v8(GST_OBJECT_NAME(factory)));
  OBJECT_SET(output, "rank", Nan::New(gst_plugin_feature_get_rank(GST_PLUGIN_FEATURE(factory))));

  GType type = G_OBJECT_TYPE(element);
  v8::Local<v8::Array> hierarchy = Nan::New<v8::Array>(count_hierarchy_depth(type));
  OBJECT_SET(output, "hierarchy", hierarchy);

  add_hierarchy(type, hierarchy);
  add_factory_details(factory, output);
  add_pad_templates(feature, factory, output);
  add_uri_handler_info(element, output);
  add_preset_list(element, output);

  gst_object_unref(element);
  //gst_object_unref(factory);
}

void Inspect(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() < 1) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  Nan::Utf8String pluginName(info[0]);
  GstPlugin *plugin = gst_registry_find_plugin(gst_registry_get(), *pluginName);

  if (!plugin) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  v8::Local<v8::Object> output = Nan::New<v8::Object>();
  OBJECT_SET(output, "name", chararray_to_v8(gst_plugin_get_name(plugin)));
  OBJECT_SET(output, "description", chararray_to_v8(gst_plugin_get_description(plugin)));

  // can be null
  OBJECT_SET(output, "filename", chararray_to_v8(gst_plugin_get_filename(plugin)));
  OBJECT_SET(output, "releaseDate", chararray_to_v8(gst_plugin_get_release_date_string(plugin)));
  
  OBJECT_SET(output, "version", chararray_to_v8(gst_plugin_get_version(plugin)));
  OBJECT_SET(output, "license", chararray_to_v8(gst_plugin_get_license(plugin)));
  OBJECT_SET(output, "source", chararray_to_v8(gst_plugin_get_source(plugin)));
  OBJECT_SET(output, "binaryPackage", chararray_to_v8(gst_plugin_get_package(plugin)));
  OBJECT_SET(output, "originUrl", chararray_to_v8(gst_plugin_get_origin(plugin)));
  OBJECT_SET(output, "backlisted", Nan::New(GST_OBJECT_FLAG_IS_SET(plugin, GST_PLUGIN_FLAG_BLACKLISTED)));

  GList *features, *orig_features;
  orig_features = features =
          gst_registry_get_feature_list_by_plugin(gst_registry_get(),
          gst_plugin_get_name(plugin));

  gst_object_unref(plugin);

  v8::Local<v8::Array> arr = Nan::New<v8::Array>(count_features(features));
  OBJECT_SET(output, "features", arr);

  for (unsigned long i = 0; features != NULL;) {
    if (features->data == NULL) {
      continue;
    }

    GstPluginFeature *feature = GST_PLUGIN_FEATURE(features->data);
    v8::Local<v8::Object> featureObject = Nan::New<v8::Object>();

    if (GST_IS_ELEMENT_FACTORY(feature)) {
      process_element_factory(feature, featureObject);
    }

    ARRAY_SET(arr, i, featureObject);
    features = g_list_next(features);
    // gst_object_unref(feature);
    i++;
  }
  
  gst_plugin_feature_list_free(orig_features);
  info.GetReturnValue().Set(output);
}

void init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE exports) {
  gst_init(NULL, NULL);
  v8::Local<v8::Context> context = exports->CreationContext();

  exports->Set(context,
               Nan::New("getPlugins").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(GetPlugins)
                   ->GetFunction(context)
                   .ToLocalChecked());

  exports->Set(context,
               Nan::New("inspect").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(Inspect)
                   ->GetFunction(context)
                   .ToLocalChecked());
}

NODE_MODULE(gst_inspect, init);
