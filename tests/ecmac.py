import sys
import json
import base64

suites = ['06', '07', '08', '09', '10', '11', '12', '13', '14', '15']

blacklists = {
    '10': []
}

def black_content(s):
    return ' in ' in s or 'LABEL' in s

num_cases = 0
def render_case(s):
    global num_cases
    decl = 'static const char c%d[] = ' % num_cases
    num_cases = num_cases + 1
    data = ','.join(['%#x' % ord(c) for c in s] + ['0'])
    return decl + '{' + data + '};'

def render_suite(suite):
    j = json.load(file("tests/ecma262_suites/ch%s.json" % suite))
    bl = blacklists.get(suite, [])
    return [render_case(base64.decodestring(i['code']))
            for n, i in enumerate(j['testsCollection']['tests'])
            if n not in bl and not black_content(base64.decodestring(i['code'])) and not i.has_key('negative')]


cases = []
for s in suites:
    cases.extend(render_suite(s))

print '\n'.join(cases)

print "static const char *ecmac_cases[] = {"
print ','.join(['c%d' % i for i in range(len(cases))])
print '};'
