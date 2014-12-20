import sys
import json
import base64

suites = ['06', '07', '08', '09', '10', '11', '12', '13', '14', '15']

blacklists = {
    '10': []
}

def black_content(s):
    return ' in ' in s or 'LABEL' in s

def render_suite(suite):
    j = json.load(file("tests/ecma262_suites/ch%s.json" % suite))
    bl = blacklists.get(suite, [])
    return ['"%s"' % base64.decodestring(i['code']).replace(chr(92), r'\\').replace("??", r"?\?").replace('\r\n', '\n').replace('\n', '\\n').replace('"', r'\"').strip("'")
            for n, i in enumerate(j['testsCollection']['tests'])
            if n not in bl and not black_content(base64.decodestring(i['code'])) and not i.has_key('negative')]

print '#pragma GCC diagnostic push'
print '#pragma GCC diagnostic ignored "-Woverlength-strings"'
print "const char *ecmac_cases[] = {\n ",
cases = []
for s in suites:
    cases.extend(render_suite(s))
print ',\n  '.join(cases)
print '};'
print '#pragma GCC diagnostic pop'
