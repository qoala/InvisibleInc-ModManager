#! /usr/bin/env python3

import argparse
import os.path
import sys

from config import Config
import actions
import config_parser


def list_downloaded_fn(config:Config, args: argparse.Namespace) -> None:
    actions.ListDownloadedAction(config).run()


def list_installed_fn(config:Config, args: argparse.Namespace) -> None:
    actions.ListInstalledAction(config).run()


def install_fn(config:Config, args: argparse.Namespace) -> None:
    actions.InstallFromSpecAction(config).run(sys.stdin)


def install_all_fn(config:Config, args: argparse.Namespace) -> None:
    actions.InstallAllAction(config).run()


def createParser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Invisible Inc Mod Manager.')
    parser.add_argument('--config', default='iimodman.conf')
    subparsers = parser.add_subparsers(required=True, dest='command')
    subparsers.add_parser('list-downloaded', aliases=['list', 'l']).set_defaults(func=list_downloaded_fn)
    subparsers.add_parser('list-installed', aliases=['li']).set_defaults(func=list_installed_fn)
    subparsers.add_parser('install').set_defaults(func=install_fn)
    subparsers.add_parser('install-all').set_defaults(func=install_all_fn)
    return parser

def main() -> None:
    parser = createParser()
    args = parser.parse_args()
    config = config_parser.read_config(os.path.expanduser(args.config))
    args.func(config, args)

if __name__ == '__main__':
    main()
