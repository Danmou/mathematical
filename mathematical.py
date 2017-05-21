from modules import validator
import re

DEFAULT_OPTS = {
  "ppi": 72.0,
  "zoom": 1.0,
  "base64": False,
  "maxsize": 0,
  "format": "svg",
  "delimiter": ["dollar", "double"]
}

XML_HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"

MATH_MATCH = re.compile(r"<math xmlns.+?</math>", re.MULTILINE)

class Process:
  __init__ = process.initialize
  process = process.process

class Mathematical:

  __version__ = '1.6.9'

  def __init__(self, options = {}):
    self.config = DEFAULT_OPTS.copy()
    self.config.update(options)

    validator.validate_config(self.config)

    self.config["formatInt"] = validator.FORMAT_TYPES.index(self.config["format"])

    if type(self.config["delimiter"]) is str:
      self.config["delimiter"] = configuration.delimiters[self.config["delimiter"]]
    else:
      self.config["delimiter"] = sum([configuration.delimiters[d] for d in self.config["delimiter"]])

    self.processer = Process()

  def parse(self, maths):
    maths = validator.validate_content(maths)
    result_data = self.processer.process(maths, validator.RENDER_TYPES.index("parse"))
    return result(result_data)

  # Alias
  render = parse

  def filter(self, maths):
    maths = validator.validate_content(maths)
    result_data = self.processer.process(maths, validator.RENDER_TYPES.index("filter"))
    return result(result_data)

  def text_filter(self, maths):
    maths = validator.validate_content(maths)
    widths = []
    heights = []
    result_data = self.processer.process(maths, validator.RENDER_TYPES.index("text_filter"))
    # TODO: can/should be optimized to not do two calls here, but I am thinking
    # about moving to Rust and don't have time to write safe C...
    if result_data["data"] and self.config["format"] != "mathml":
      def repl(match):
        result = self.processer.process(match, validator.RENDER_TYPES.index("parse"))
        widths.append(result["width"])
        heights.append(result["height"])
        return result["data"]
      result_data["data"] = MATH_MATCH.sub(repl, result_data["data"])

      result_data["width"] = widths
      result_data["height"] = heights

    return result(result_data)

  def strict_filter(self, maths):
    maths = validator.validate_content(maths)
    result_data = self.processer.process(maths, validator.RENDER_TYPES.index("strict_filter"))
    return result(result_data)

  def result(self, result_data):
    if not type(result_data) in [dict, list]:
      raise RuntimeError()

    if type(result_data) is list:
      return [__format_data(d) for d in result_data]
    else:
      return __format_data(result_data)

  def __format_data(self, result_dict):
    # we passed in an array of math, and found an unprocessable element
    if "exception" in result_dict:
      return result_dict

    if self.config["format"] == "svg":
      # remove starting <?xml...> tag
      result_dict["data"] = XML_HEADER.sub('', result_dict["data"])
      if self.config["base64"]:
        result_dict["data"] = __svg_to_base64(result_dict["data"])

      return result_dict
    elif self.config["format"] in ["png", "mathml"]: # do nothing with these...for now?
      return result_dict

  def __svg_to_base64(self, contents):
    return "data:image/svg+xml;base64,{}".format(base64.b64encode(contents.encode('utf-8')).decode('ascii'))
