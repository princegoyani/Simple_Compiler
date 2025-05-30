# Simple Compiler

A recursive-descent compiler for a small polynomial language, performing syntax & semantic checks and executing polynomial evaluations.

## Overview

This compiler:
- Parses a language with `TASKS`, `POLY`, `EXECUTE` and `INPUTS` sections.
- Detects syntax errors and four categories of semantic errors.
- Executes polynomial evaluations, handling inputs/outputs and assignment statements.

Based on the specification for CSE340 Spring 2025 – Project 1 :contentReference[oaicite:0]{index=0}.

## Features

- **Syntax Checking**: Reports `SYNTAX ERROR !!!!!&%!!` on malformed input.
- **Semantic Checking**:  
  - Duplicate polynomial declarations  
  - Invalid monomial names  
  - Undeclared polynomial evaluations  
  - Wrong number of arguments  
- **Execution**:  
  - Reads variables via `INPUT` statements  
  - Evaluates polynomials (supports multivariate)  
  - Outputs results with `OUTPUT` statements

## Tech Stack

- C++11  
- GNU GCC on Ubuntu 22.04  
- Gradescope for automated testing

## Repo Structure

