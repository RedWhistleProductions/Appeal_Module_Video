"""Install the FFmpeg LGPL shared SDK inside this plugin project."""
from pathlib import Path
import hashlib,json,shutil,urllib.request,zipfile
base=Path(__file__).resolve().parents[1]
deps=base/'Appeal_FFmpeg_Video/.deps';deps.mkdir(parents=True,exist_ok=True)
release=json.load(urllib.request.urlopen('https://api.github.com/repos/BtbN/FFmpeg-Builds/releases/tags/latest',timeout=30))
asset=next(a for a in release['assets'] if a['name']=='ffmpeg-n8.1-latest-win64-lgpl-shared-8.1.zip')
archive=deps/asset['name']
with urllib.request.urlopen(asset['browser_download_url'],timeout=60) as response,archive.open('wb') as target:
    shutil.copyfileobj(response,target)
sha=hashlib.sha256(archive.read_bytes()).hexdigest()
if asset.get('digest') and asset['digest']!='sha256:'+sha: raise RuntimeError('FFmpeg archive digest mismatch')
output=deps/'ffmpeg';output.mkdir(exist_ok=True)
with zipfile.ZipFile(archive) as z:
    for item in z.infolist():
        parts=Path(item.filename).parts[1:]
        if not parts: continue
        target=output.joinpath(*parts).resolve()
        if not target.is_relative_to(output.resolve()): raise RuntimeError('Unsafe archive path')
        if item.is_dir(): target.mkdir(parents=True,exist_ok=True)
        else:
            target.parent.mkdir(parents=True,exist_ok=True)
            with z.open(item) as source,target.open('wb') as dest: shutil.copyfileobj(source,dest)
(output/'APPEAL_SOURCE.json').write_text(json.dumps({'release':release['tag_name'],'asset':asset['name'],'url':asset['browser_download_url'],'sha256':sha},indent=2))
root=base.parents[2]
for dll in (output/'bin').glob('*.dll'): shutil.copy2(dll,root/dll.name)
print('FFmpeg SDK installed:',output)
