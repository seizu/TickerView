import os
import re
from SCons.Script import DefaultEnvironment

Import("env")

# ---------- minify functions ----------
def minify_html(html_code):
    html_code = re.sub(r'<!--.*?-->', '', html_code, flags=re.S)
    html_code = re.sub(r'\s+', ' ', html_code)
    return html_code.strip()

def minify_css(css_code):
    css_code = re.sub(r'/\*.*?\*/', '', css_code, flags=re.S)
    css_code = re.sub(r'\s+', ' ', css_code)
    css_code = re.sub(r'\s*([{}:;,])\s*', r'\1', css_code)
    return css_code.strip()

def minify_js(js_code):
    js_code = re.sub(r'//.*?\n|/\*.*?\*/', '', js_code, flags=re.S)
    js_code = re.sub(r'\s+', ' ', js_code)
    return js_code.strip()

# ---------- Get config FIRST ----------
html_input = env.GetProjectOption("custom_html_input", "src/html")
html_output = env.GetProjectOption("custom_html_output", "lib/WebPrefs/page.h")

print(f"HTML Input: {html_input}")
print(f"HTML Output: {html_output}")

# ---------- Now define pagedata with actual paths ----------
pagedata = {
    "outfile": f"{html_output}/page.h",
    "infiles": [
        f"{html_input}/head.html",      "head",          "",                                "",
        f"{html_input}/done.html",      "done_body",     "</head>",                         "</html>",
        f"{html_input}/settings.html",  "settings_body", "</head>",                         "</html>",
        f"{html_input}/script.js",      "javascript",    '<script type="text/javascript">', "</script>",
        f"{html_input}/style.css",      "style",         "<style>",                         "</style>"
    ]
}

def before_build():
    print("minify.py is running ...")
    outfile_path = pagedata['outfile']
    infiles = pagedata['infiles']

    os.makedirs(os.path.dirname(outfile_path), exist_ok=True)

    with open(outfile_path, 'w') as outfile:
        for i in range(0, len(infiles), 4):
            infile, var_name, pre_txt, post_txt = infiles[i], infiles[i+1], infiles[i+2], infiles[i+3]

            if not os.path.exists(infile):
                print(f"File not found: {infile}")
                continue

            with open(infile, 'r') as file:
                content = file.read()

            if infile.endswith('.html'):
                minified_content = minify_html(content)
            elif infile.endswith('.css'):
                minified_content = minify_css(content)
            elif infile.endswith('.js'):
                minified_content = minify_js(content)
            else:
                print(f"unknown type: {infile}")
                continue

            pre = f'static const char {var_name}[] PROGMEM = R"rawliteral({pre_txt}'
            post = f'{post_txt})rawliteral";'

            outfile.write(f'{pre}{minified_content}{post}\n')
            print(f'[OK] {infile} was minified and inserted into {outfile_path}.')

before_build()