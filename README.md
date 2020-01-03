INSTALLATION
============
Support only linux yet. Contributions are welcome.  
System dependencies : `libgstreamer-1.0-dev libgstreamer-plugins-base1.0-dev`

To install run `npm i node-gstreamer-tools`

## Examples

### Plugins inspection

```js
const gst = require('node-gstreamer-tools');

const plugins = gst.getPlugins();
const pluginDetails = gst.inspect(plugins[0]);

console.log(plugins, pluginDetails);

```

### Media inspection
```js
const gst = require('node-gstreamer-tools');

gst
  .discover("file://<media path>", 60) // 60 = timeout in seconds
  .then(mediaInfos => {
    console.log(mediaInfos);
  })
  .catch(e => {
    console.log(e);
  })
;

// or 

gst
  .discover("http(s)://<url>", 10)
  .then(mediaInfos => {
    console.log(mediaInfos);
  })
  .catch(e => {
    console.log(e);
  })
;
```


## Constants

### features[].padTemplates[].direction
https://gstreamer.freedesktop.org/documentation/gstreamer/gstpad.html#GstPadDirection   

Value | Description
------|------------
0     | unknown
1     | src
2     | sink


### features[].padTemplates[].presence

https://gstreamer.freedesktop.org/documentation/gstreamer/gstpadtemplate.html?gi-language=c#GstPadPresence

Value | Description
------|------------
0     | Always
1     | Sometimes
2     | Request


### features[].uriHandler

https://gstreamer.freedesktop.org/documentation/gstreamer/gsturihandler.html?gi-language=c#GstURIType

Value | Description
------|------------
0     | The URI direction is unknown
1     | The URI is a consumer
2     | The URI is a producer

https://github.com/GStreamer/gstreamer/blob/master/tools/gst-inspect.c#L769