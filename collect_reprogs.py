#!/usr/bin/env python3

import argparse
import glob
import filecmp
import logging
import os
import shutil


def main():
    parser = argparse.ArgumentParser(description='Collect C reproducers programs from subdirs.')
    parser.add_argument('directory', type=str,
                        help='Folder to be searched for reprogs.')
    parser.add_argument('--output', '-o', required=False, default="./linux",
                        help='Output folder where to store. default ./linux')
    parser.add_argument('--collect_all', '-c', required=False, default=False, action='store_true',
                        help='There might be more than one C reproducers per bug. Use this if you want to collect them all. \
                            By default only the first one is added to the output dir.')
    args = parser.parse_args()

    workdir = os.path.join(args.directory, "")
    logging.debug(workdir)

    for file in glob.iglob(workdir + '**/*.cprog', recursive=True):
        logging.debug(file)
        file_path = file.split('/')
        has_id_index = len(file_path) - 2

        candidate_name = file_path[has_id_index]
        candidate_file_name = os.path.join(args.output, candidate_name + ".c")
        index = 1
        already_exist = False
        while os.path.exists(candidate_file_name):
            if filecmp.cmp(candidate_file_name, file, shallow=False):
                already_exist = True
                break
            candidate_file_name = os.path.join(args.output, candidate_name + "_" + str(index).rjust(3, '0') + ".c")
            index += 1

        if not already_exist and args.collect_all:
            logging.info("Added new reprog: " + candidate_file_name)
            if not os.path.exists(args.output):
                os.mkdir(args.output)
            shutil.copy(file, candidate_file_name)
        else:
            logging.info("Reproducer: " + file + " already exists.")


if __name__ == "__main__":
    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)
    main()
