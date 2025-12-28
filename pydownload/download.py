import os
import json
import aiohttp
import asyncio
import zipfile  # 建议将导入放在文件顶部

MAX_CONCURRENT_FILES = 8  # 同时下载的文件数
CHUNK_SIZE = 1024 * 1024  # 1MB 分块


async def download_file(session, file_info, target_dir):
    url = file_info['url']
    filename = file_info['filename']
    unzip = file_info.get('unzip', False)
    save_path = os.path.join(target_dir, filename)

    # 1. 检查文件是否存在（防重复下载）
    if os.path.exists(save_path):
        print(f"⏭️  跳过 (已存在): {filename}")
        return

    # 2. 确认不存在后，提示开始下载
    print(f"⬇️  正在下载: {filename}")

    try:
        async with session.get(url) as resp:
            resp.raise_for_status()
            with open(save_path, 'wb') as f:
                async for chunk in resp.content.iter_chunked(CHUNK_SIZE):
                    f.write(chunk)

        print(f"✅ 下载完成: {filename}")

        # 3. 如果需要解压
        if unzip and save_path.endswith('.zip'):
            try:
                with zipfile.ZipFile(save_path, 'r') as zip_ref:
                    zip_ref.extractall(target_dir)
                print(f"📦 解压完成: {filename}")
            except zipfile.BadZipFile:
                print(f"⚠️  解压失败 (文件损坏): {filename}")
            except Exception as e:
                print(f"⚠️  解压出错: {e}")

    except Exception as e:
        # 4. 下载失败处理：删除可能产生的半成品文件，防止下次误判为已存在
        if os.path.exists(save_path):
            try:
                os.remove(save_path)
            except OSError:
                pass
        print(f"❌ 下载失败 {url}: {e}")


async def process_json(json_path, session):
    target_dir = os.path.dirname(json_path)
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            data = json.load(f)

        # 创建任务列表
        tasks = [download_file(session, file_info, target_dir) for file_info in data.get('files', [])]

        if tasks:
            await asyncio.gather(*tasks)

    except Exception as e:
        print(f"❌ 处理 JSON 失败 {json_path}: {e}")


async def main():
    # 获取 resources 目录的绝对路径
    base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'resources'))

    json_files = []
    # 递归查找 json 文件
    for root, _, files in os.walk(base_dir):
        for file in files:
            if file.lower().endswith('.json'):
                json_files.append(os.path.join(root, file))

    if not json_files:
        print("未在 resources 目录下找到 JSON 配置文件。")
        return

    print(f"发现 {len(json_files)} 个配置文件，准备处理...")

    connector = aiohttp.TCPConnector(limit_per_host=MAX_CONCURRENT_FILES)
    async with aiohttp.ClientSession(connector=connector) as session:
        tasks = [process_json(jf, session) for jf in json_files]
        await asyncio.gather(*tasks)


if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n⛔ 用户手动停止程序")