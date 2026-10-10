import os
import sys
top = '.'
out = 'build'
def options(ctx):
    ctx.load('pebble_sdk')
def configure(ctx):
    ctx.load('pebble_sdk')
def build(ctx):
    ctx.load('pebble_sdk')
    fit_check=ctx.path.find_node('tools/validate_time_fit.py').abspath()
    if ctx.exec_command([sys.executable,fit_check]) != 0:
        ctx.fatal('Time-fit validation failed: check numeral bounds and spacing')
    binaries = []
    for p in ctx.env.TARGET_PLATFORMS:
        ctx.set_env(ctx.all_envs[p])
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        theme=int(os.environ.get('POC_THEME','-1'))
        if theme not in range(-1,4): ctx.fatal('POC_THEME must be -1..3')
        ctx.pbl_program(source=ctx.path.ant_glob('src/c/**/*.c'), target=app_elf,
                        defines=['POC_THEME=%d'%theme,'POC_HOUR=%d'%int(os.environ.get('POC_HOUR','-1')),'POC_MINUTE=%d'%int(os.environ.get('POC_MINUTE','42'))])
        binaries.append({'platform':p, 'app_elf':app_elf})
    ctx(features='subst',source='package.json',target='js/package.json',is_copy=True)
    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries,js=ctx.path.ant_glob(['src/pkjs/**/*.js','src/pkjs/**/*.json']),js_entry_file='src/pkjs/index.js')
