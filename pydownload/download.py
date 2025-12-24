import os
import json
import aiohttp
import asyncio

MAX_CONCURRENT_FILES = 8  # 同时下载的文件数
CHUNK_SIZE = 1024 * 1024  # 1MB 分块

async def download_file(session, file_info, target_dir):
    url = file_info['url']
    filename = file_info['filename']
    unzip = file_info.get('unzip', False)
    save_path = os.path.join(target_dir, filename)

    try:
        async with session.get(url) as resp:
            resp.raise_for_status()
            with open(save_path, 'wb') as f:
                async for chunk in resp.content.iter_chunked(CHUNK_SIZE):
                    f.write(chunk)
        print(f"Downloaded: {save_path}")

        if unzip and save_path.endswith('.zip'):
            import zipfile
            with zipfile.ZipFile(save_path, 'r') as zip_ref:
                zip_ref.extractall(target_dir)
            print(f"Unzipped: {save_path}")

    except Exception as e:
        print(f"Failed to download {url}: {e}")

async def process_json(json_path, session):
    target_dir = os.path.dirname(json_path)
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        tasks = [download_file(session, file_info, target_dir) for file_info in data.get('files', [])]
        if tasks:
            await asyncio.gather(*tasks)
    except Exception as e:
        print(f"Failed to process {json_path}: {e}")

async def main():
    base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'resources'))
    json_files = []
    for root, _, files in os.walk(base_dir):
        for file in files:
            if file.lower().endswith('.json'):
                json_files.append(os.path.join(root, file))

    connector = aiohttp.TCPConnector(limit_per_host=MAX_CONCURRENT_FILES)
    async with aiohttp.ClientSession(connector=connector) as session:
        tasks = [process_json(jf, session) for jf in json_files]
        await asyncio.gather(*tasks)

if __name__ == '__main__':
    asyncio.run(main())
