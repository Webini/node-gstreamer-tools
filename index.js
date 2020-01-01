const inspect = require('./inspect');
const discover = require('./discover');

module.exports = {
  inspect: inspect.inspect,
  getPlugins: inspect.getPlugins,
  discover,
};
