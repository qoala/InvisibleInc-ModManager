
import configparser
import os.path

from config import Config

def read_config(filename: str) -> Config:
    raw_config = configparser.ConfigParser()
    raw_config.read(filename)

    return Config(
            install_path=os.path.expanduser(raw_config.get('iimodman', 'InstallPath')),
            download_path=os.path.expanduser(raw_config.get('iimodman', 'DownloadPath')))
