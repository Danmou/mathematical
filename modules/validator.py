from modules import configuration

FORMAT_TYPES = ["svg", "png", "mathml"]
RENDER_TYPES = ["parse", "filter", "text_filter", "strict_filter"]

def validate_config(config):
  if not type(config["maxsize"]) is int:
    raise TypeError('maxsize must be an integer!')
  if config["maxsize"] < 0:
    raise TypeError('maxsize cannot be less than 0!')
  if not type(config["format"]) is str:
    raise TypeError('format must be a string!')
  if not config["format"] in FORMAT_TYPES:
    raise TypeError('format type must be one of the following formats: {}'.format(FORMAT_TYPES.join(', ')))
  if type(config["delimiter"]) is str:
    if not config["delimiter"] in configuration.delimiters:
      raise TypeError('delimiter type does not exist: {}'.format(config["delimiter"]))
  elif type(config["delimiter"]) is list:
    if not config["delimiter"]:
      config["delimiter"] = [None]

    for delim in config["delimiter"]:
      if not delim in configuration.delimiters.keys():
        raise TypeError('delimiter type does not exist: {}'.format(delim))
  else:
    raise TypeError('delimiter type must be a valid string or list of strings')

def validate_content(maths):
  if type(maths) is list:
    return [validate_string(m) for m in maths]
  else:
    return validate_string(maths)

def validate_string(maths):
  if not type(maths) is str:
    raise TypeError('input must be a string!')
  return maths.strip()
