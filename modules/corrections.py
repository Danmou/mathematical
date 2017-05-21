import re

def apply_corrections(maths):
  return adjust_lt_gt(maths)

# from the itex website: http://bit.ly/1Et74ed
# It is possible (though probably not recommended) to insert MathML markup
# inside itex equations. So "<" and ">" are significant.
# To obtain a less-than or greater-than sign, you should use \lt or \gt, respectively.
def adjust_lt_gt(maths):
  return re.sub(r'>', r'\gt', re.sub(r'<', r'\lt', maths))
