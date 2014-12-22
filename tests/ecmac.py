import sys
import json
import base64

suites = ['06', '07', '08', '09', '10', '11', '12', '13', '14', '15']

blacklists = {
    '10': []
}

def black_content(s):
    return False

def render_suite(suite):
    j = json.load(file("tests/ecma262_suites/ch%s.json" % suite))
    bl = blacklists.get(suite, [])
    return [base64.decodestring(i['code']).strip("'")
            for n, i in enumerate(j['testsCollection']['tests'])
            if n not in bl and not black_content(base64.decodestring(i['code'])) and not i.has_key('negative')]

cases = []
for s in suites:
    cases.extend(render_suite(s))
print '\0'.join(cases),
