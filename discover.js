const util = require('util');
const bindings = require('bindings')('gst-discover');
const discoverAsync = util.promisify(bindings.discover);

module.exports = function(filepath, secTimeout = 10) {  
  return discoverAsync(filepath, secTimeout);
};
