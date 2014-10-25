#!/usr/bin/env python
#coding:utf-8

from BaseAI import BaseAI
import MarvinAI

class MarvinAI(BaseAI):
        def __init__(self):
                MarvinAI.initialize()
	def getMove(self, grid):
                m = MarvinAI.evaluate(grid.map)
                return m
