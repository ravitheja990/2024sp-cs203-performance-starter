#!/usr/bin/env python

import click
import os
import json
import re
import math
import pandas as pd
#from csvtools import qcsv
#from CSE142L.jextract import extract as qjson

@click.command()
@click.option("--submission", required=True,  type=click.Path(exists=True), help="Test directory")
@click.option("--results", required=True, type = click.File(mode="w"), help="Where to put results")
def autograde(submission=None, results=None):

    target_speedup = 8.0
    df = pd.read_csv("autograde.csv", sep=",");
    speedup = round(df.at[0,"ET"]/df.at[1,"ET"]);
#    speedup = round(float(qcsv(os.path.join(submission, "autograde.csv"), field="speedup",where=["function"], _is=["sum_of_locations_solution"])), 2)
#    failures = qjson(json.load(open(os.path.join(submission, "regressions.json"))), ["testsuites", 0, "failures"])
    correct = (df.at[0,"answer"] == df.at[1,"answer"])
    score = min(round(speedup/target_speedup*100.0, 2),100.0)

    output = "tests passed" if correct else "Your code is incorrect"
    # https://gradescope-autograders.readthedocs.io/en/latest/specs/#output-format
    json.dump(dict(output="The autograder ran.",
                   visibility="visible",
                   stdout_visibility="visible",
                   tests=[dict(score=100 if correct else 0,
                               max_score=100,
                               number="1",
                               output=output,
                               tags=[],
                               visibility="visible"),
                          dict(score=score if correct else 0,
                               max_score=100,
                               number="2",
                               output=f"Your speedup is {speedup:2.2f}x.  The target speedup is {target_speedup}x. Your score is the minimum of 100 and {speedup:2.2f}/{target_speedup}*100 = {score}" if correct else "Your code is incorrect, so speedup is meaningless.",
                               tags=[],
                               visibility="visible")
                   ],
                   leaderboard=[
                       dict(name="speedup", value=speedup)
                   ]
        ), results, indent=4)
        
if __name__== "__main__":
    autograde()
