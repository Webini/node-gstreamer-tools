#include "Discover.h"

Discover::Discover(Nan::Callback* callback, unsigned int timeout, const char *filepath)
  : Nan::AsyncWorker(callback), timeout(timeout), error(NULL), dc(NULL), info(NULL), gerr(NULL) {
  this->filepath = g_strdup(filepath);
}

Discover::~Discover() {
  clean();
  g_free((gpointer)filepath);
}

void Discover::clean() {
  if (dc != NULL) {
    g_object_unref(dc);
    dc = NULL;
  }
  if (gerr != NULL) {
    g_clear_error(&gerr);
    gerr = NULL;
  }
  if (info != NULL) {
    gst_discoverer_info_unref(info);
    info = NULL;
  }
}

void Discover::Execute() {
  dc = gst_discoverer_new(timeout *GST_SECOND, &gerr);

  if (G_UNLIKELY(dc == NULL)) {
    error = "Cannot initialize discoverer";
    return;
  }

  info = gst_discoverer_discover_uri(dc, filepath, &gerr);
}

void Discover::addAudioInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output) {
  GstDiscovererAudioInfo *audioInfo = (GstDiscovererAudioInfo *)info;
  const GstTagList *tags = gst_discoverer_stream_info_get_tags(info);

  if (tags != NULL) {
    v8::Local<v8::Object> tagsObject = Nan::New<v8::Object>();
    gst_tag_list_foreach(tags, gst_tags_to_v8_iterate, (gpointer)&tagsObject);
    OBJECT_SET(output, "tags", tagsObject);
  }

  OBJECT_SET(output, "streamId", chararray_to_v8(gst_discoverer_stream_info_get_stream_id(info)));
  OBJECT_SET(output, "language", chararray_to_v8(gst_discoverer_audio_info_get_language(audioInfo)));
  OBJECT_SET(output, "channels", Nan::New(gst_discoverer_audio_info_get_channels(audioInfo)));
  OBJECT_SET(output, "sampleRate", Nan::New(gst_discoverer_audio_info_get_sample_rate(audioInfo)));
  OBJECT_SET(output, "depth", Nan::New(gst_discoverer_audio_info_get_depth(audioInfo)));
  OBJECT_SET(output, "bitrate", Nan::New(gst_discoverer_audio_info_get_bitrate(audioInfo)));
  OBJECT_SET(output, "maxBitrate", Nan::New(gst_discoverer_audio_info_get_max_bitrate(audioInfo)));
}

void Discover::addVideoInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output) {
  GstDiscovererVideoInfo *videoInfo = (GstDiscovererVideoInfo *)info;
  const GstTagList *tags = gst_discoverer_stream_info_get_tags(info);

  if (tags != NULL) {
    v8::Local<v8::Object> tagsObject = Nan::New<v8::Object>();
    gst_tag_list_foreach(tags, gst_tags_to_v8_iterate, (gpointer)&tagsObject);
    OBJECT_SET(output, "tags", tagsObject);
  }

  OBJECT_SET(output, "streamId", chararray_to_v8(gst_discoverer_stream_info_get_stream_id(info)));
  OBJECT_SET(output, "width", Nan::New(gst_discoverer_video_info_get_width(videoInfo)));
  OBJECT_SET(output, "height", Nan::New(gst_discoverer_video_info_get_height(videoInfo)));
  OBJECT_SET(output, "depth", Nan::New(gst_discoverer_video_info_get_depth(videoInfo)));
  
  v8::Local<v8::Object> framerate = Nan::New<v8::Object>();
  OBJECT_SET(framerate, "num", Nan::New(gst_discoverer_video_info_get_framerate_num(videoInfo)));
  OBJECT_SET(framerate, "denom", Nan::New(gst_discoverer_video_info_get_framerate_denom(videoInfo)));
  OBJECT_SET(output, "framerate", framerate);

  v8::Local<v8::Object> pixelAspectRatio = Nan::New<v8::Object>();
  OBJECT_SET(pixelAspectRatio, "num", Nan::New(gst_discoverer_video_info_get_par_num(videoInfo)));
  OBJECT_SET(pixelAspectRatio, "denom", Nan::New(gst_discoverer_video_info_get_par_denom(videoInfo)));
  OBJECT_SET(output, "pixelAspectRatio", pixelAspectRatio);
  
  OBJECT_SET(output, "interlaced", Nan::New(!!gst_discoverer_video_info_is_interlaced(videoInfo)));
  OBJECT_SET(output, "bitrate", Nan::New(gst_discoverer_video_info_get_bitrate(videoInfo)));
  OBJECT_SET(output, "image", Nan::New(!!gst_discoverer_video_info_is_image(videoInfo)));
  OBJECT_SET(output, "maxBitrate", Nan::New(gst_discoverer_video_info_get_max_bitrate(videoInfo)));
}

void Discover::addSubtitleInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output) {
  GstDiscovererSubtitleInfo *subInfo = (GstDiscovererSubtitleInfo *)info;
  const GstTagList *tags = gst_discoverer_stream_info_get_tags(info);
  
  if (tags != NULL) {
    v8::Local<v8::Object> tagsObject = Nan::New<v8::Object>();
    gst_tag_list_foreach(tags, gst_tags_to_v8_iterate, (gpointer)&tagsObject);
    OBJECT_SET(output, "tags", tagsObject);
  }

  OBJECT_SET(output, "streamId", chararray_to_v8(gst_discoverer_stream_info_get_stream_id(info)));
  OBJECT_SET(output, "language", chararray_to_v8(gst_discoverer_subtitle_info_get_language(subInfo)));
}

void Discover::addStreamInfo(GstDiscovererStreamInfo *info, v8::Local<v8::Object> &output) {
  if (info == NULL) {
    return;
  }

  GList *streams = NULL;
  GstCaps *caps = gst_discoverer_stream_info_get_caps(info);
  unsigned int streamSz = 0;
  unsigned int streamIt = 0;

  OBJECT_SET(output, "type", chararray_to_v8(gst_discoverer_stream_info_get_stream_type_nick(info)));

  if (caps != NULL) {
    v8::Local<v8::Object> capsObject = Nan::New<v8::Object>();

    for (unsigned long i = 0; i < gst_caps_get_size(caps); i++) {
      GstStructure *structure = gst_caps_get_structure(caps, i);
      OBJECT_SET(capsObject, "type", chararray_to_v8(gst_structure_get_name(structure)));
      gst_structure_foreach(structure, gst_structure_to_v8_value_iterate, (gpointer)&capsObject);
    }

    OBJECT_SET(output, "codec", capsObject);
  }
  
  if (GST_IS_DISCOVERER_AUDIO_INFO(info)) {
    this->addAudioInfo(info, output);
  } else if (GST_IS_DISCOVERER_VIDEO_INFO(info)) {
    this->addVideoInfo(info, output);
  } else if (GST_IS_DISCOVERER_SUBTITLE_INFO(info)) {
    this->addSubtitleInfo(info, output);
  }

  if (GST_IS_DISCOVERER_CONTAINER_INFO(info)) {
    streams = gst_discoverer_container_info_get_streams(GST_DISCOVERER_CONTAINER_INFO(info));
    for (GList *stream = streams; stream != NULL; stream = stream->next, streamSz++);
  }

  for (GstDiscovererStreamInfo *next = gst_discoverer_stream_info_get_next(info); next != NULL; next = gst_discoverer_stream_info_get_next(next), streamSz++);

  v8::Local<v8::Array> arr = Nan::New<v8::Array>(streamSz);

  if (streams != NULL) {
    for (GList *stream = streams; stream != NULL; stream = stream->next, streamIt++) {  
      v8::Local<v8::Object> infoObject = Nan::New<v8::Object>();
      GstDiscovererStreamInfo *tmpInf = (GstDiscovererStreamInfo *)stream->data;
      addStreamInfo(tmpInf, infoObject);
      ARRAY_SET(arr,streamIt, infoObject);
    }

    gst_discoverer_stream_info_list_free(streams);
  }

  for (
    GstDiscovererStreamInfo *next = gst_discoverer_stream_info_get_next(info); 
    next != NULL; 
    next = gst_discoverer_stream_info_get_next(next), streamIt++
  ) {
    v8::Local<v8::Object> infoObject = Nan::New<v8::Object>();
    addStreamInfo(next, infoObject);
    ARRAY_SET(arr, streamIt, infoObject);
  }

  if (streamSz > 0) {
    OBJECT_SET(output, "streams", arr);
  }
}

void Discover::processDc(v8::Local<v8::Object> &output) {
  GstDiscovererResult result;
  GstDiscovererStreamInfo *sinfo;

  if (info == NULL) {
    error = "Info not set";
    return;
  }

  result = gst_discoverer_info_get_result(info);
  if (result != GST_DISCOVERER_OK) {
    error = "Discoverer not ok";
    return;
  }

  sinfo = gst_discoverer_info_get_stream_info(info);
  if (sinfo == NULL) {
    error = "Cannot retrieve stream info";
    return;
  }
  
  v8::Local<v8::Object> topology = Nan::New<v8::Object>();
  const GstTagList *tags = gst_discoverer_info_get_tags(info);

  if (tags != NULL) {
    v8::Local<v8::Object> tagsObject = Nan::New<v8::Object>();
    gst_tag_list_foreach(tags, gst_tags_to_v8_iterate, (gpointer)&tagsObject);
    OBJECT_SET(output, "tags", tagsObject);
  }

  v8::Local<v8::Object> duration = Nan::New<v8::Object>();
  unsigned int times[] = { GST_TIME_ARGS(gst_discoverer_info_get_duration(info)) };
  OBJECT_SET(duration, "h", Nan::New(times[0]));
  OBJECT_SET(duration, "m", Nan::New(times[1]));
  OBJECT_SET(duration, "s", Nan::New(times[2]));
  OBJECT_SET(duration, "us", Nan::New(times[3]));

  OBJECT_SET(output, "duration", duration);
  OBJECT_SET(output, "seekable", Nan::New(!!gst_discoverer_info_get_seekable(info)));
  OBJECT_SET(output, "live", Nan::New(!!gst_discoverer_info_get_live(info)));
  OBJECT_SET(output, "topology", topology);

  addStreamInfo(sinfo, topology);
}

void Discover::HandleOKCallback() {
  Nan::HandleScope scope;
  v8::Local<v8::Object> output = Nan::New<v8::Object>();
  v8::Local<v8::Value> argv[2] = { Nan::Null(), Nan::Null() };

  processDc(output);

  if (error != NULL) {
    argv[0] = Nan::New(error).ToLocalChecked();
  } else if (gerr != NULL) {
    argv[0] = Nan::New(gerr->message).ToLocalChecked();
  } else {
    argv[1] = output;
  }

  clean();
  callback->Call(2, argv, async_resource);
}