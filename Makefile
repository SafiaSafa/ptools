.PHONY: help clean clean-build clean-pyc clean-test lint test docs 

# Name of the docker image.
DOCKER_IMAGE = ptools:dev

help:
	@echo "usage: make <command>"
	@echo ""
	@echo "Available commands:"
	@echo "    help - show this help"
	@echo "    clean - remove all build, test, coverage and Python artifacts"
	@echo "    clean-build - remove all build artifacts"
	@echo "    clean-pyc - remove all Python artifacts"
	@echo "    clean-test - remove test and coverage artifacts"
	@echo "    lint - check style with flake8"
	@echo "    test - run tests with default Python"
	@echo "    docs - generate Sphinx HTML documentation"
	@echo "    build - build the package"
	@echo "    install - install the package to the active Python's site-packages"
	@echo "    docker-build - build a docker container for ptools"
	@echo "    docker-test - use the docker container ptools:dev to run unit tests"


clean: clean-build clean-pyc clean-test


clean-build:
	rm -fr build/
	rm -fr dist/
	rm -fr .eggs/
	rm -f bindings/_ptools.cpp
	rm -f PyAttract/cgopt.c
	find . -name '*.egg-info' -exec rm -fr {} +
	find . -name '*.egg' -exec rm -f {} +


clean-pyc:
	find . -name '*.pyc' -exec rm -f {} +
	find . -name '*.pyo' -exec rm -f {} +
	find . -name '*~' -exec rm -f {} +
	find . -name '__pycache__' -exec rm -fr {} +


clean-test:
	rm -f Tests/cpp/runner.cpp
	rm -f Tests/cpp/ptoolstest.bin
	rm -rf .cache


lint:
	flake8 --ignore=E501 tests Heligeom PyAttract
	flake8 --ignore=F401 ptools  # Ignore the 'imported but unused' warnings


test: build
	$(MAKE) -C Tests


docs:
	$(MAKE) -C docs clean
	$(MAKE) -C docs html


build: clean
	python setup.py build


install:
	python setup.py install


docker-build:
	docker build -t $(DOCKER_IMAGE) ./docker


docker-test:
	docker run --rm -v $(shell pwd):/src/ptools $(DOCKER_IMAGE) test


docker-run:
	docker run --name ptools_interactive -it -v $(shell pwd):/src/ptools $(DOCKER_IMAGE)
	
