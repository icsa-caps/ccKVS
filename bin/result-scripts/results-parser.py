#!/usr/bin/python

import sys, os, ntpath, getopt

"""
========
Parser 4 aggregated over time results
========
"""
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def printUnderlined(string):
    print bcolors.UNDERLINE + string + bcolors.ENDC,

def printYellow(string):
    print bcolors.WARNING + string + bcolors.ENDC,


def printCsvLineFromArray(string, array):
    print string,":",
    for i in xrange(len(array)):
        if i == 0:
            print array[i],
        else:
            print ",",
            print ('%.2f' % array[i]),
    print ""

def printCSVFrom2DArray(string, array):
    for i in xrange(len(array)):
        printCsvLineFromArray(string, array[i])

def array_sum(first_element, src_2d_array, dst_array):
    for i in xrange(len(src_2d_array)):
        for j in xrange(len(src_2d_array[i])):
            if j == 0:
                dst_array[j] = first_element
            else:
                dst_array[j] += src_2d_array[i][j]

def array_var(first_element, src_2d_array, dst_array): # in percentage
    min_array = [ 0 for x in xrange(len(src_2d_array[0]))]
    max_array = [ 0 for x in xrange(len(src_2d_array[0]))]
    for i in xrange(len(src_2d_array)):
        for j in xrange(len(src_2d_array[i])):
            if i == 0:
                min_array[j] = src_2d_array[i][j]
            elif min_array[j] > src_2d_array[i][j]:
                min_array[j] = src_2d_array[i][j]
            if max_array[j] < src_2d_array[i][j]:
                max_array[j] = src_2d_array[i][j]
    for i in xrange(len(max_array)):
        if i == 0:
            dst_array[i] = first_element
        else:
            if max_array[i] != 0:
                dst_array[i] = 100 * (max_array[i] - min_array[i]) / max_array[i]

def array_avg(first_element, sum_array, avg_array, counter):
    for i in xrange(len(sum_array)):
        if i == 0:
            avg_array[i] = first_element
        else:
            if sum_array[i] != 0:
                avg_array[i] = sum_array[i] / counter


class Machine:
    def __init__(self, id):
        self.id = id
        self.clients = [] #2D
        self.workers = []

    def calc_values(self):
        self.sum_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.sum_workers = [ 0 for x in xrange(len(self.workers[0]))]
        self.avg_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.avg_workers = [ 0 for x in xrange(len(self.workers[0]))]
        self.var_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.var_workers = [ 0 for x in xrange(len(self.workers[0]))]

        array_sum(self.id, self.clients, self.sum_clients)
        array_sum(self.id, self.workers, self.sum_workers)
        array_var(self.id, self.clients, self.var_clients)
        array_var(self.id, self.workers, self.var_workers)
        array_avg(self.id, self.sum_workers, self.avg_workers, len(self.workers))
        array_avg(self.id, self.sum_clients, self.avg_clients, len(self.clients))

        
            
    def printAll(self):
        #printCSVFrom2DArray("clients",self.clients)   
        #printCSVFrom2DArray("workers",self.workers)   
        self.calc_values()
        printCsvLineFromArray("machine_avg_workers ", self.avg_workers)
        printCsvLineFromArray("machine_%var_workers", self.var_workers)
        printCsvLineFromArray("machine_sum_workers ", self.sum_workers)
        printCsvLineFromArray("machine_avg_clients ", self.avg_clients)
        printCsvLineFromArray("machine_%var_clients", self.var_clients)
        printCsvLineFromArray("machine_sum_clients ", self.sum_clients)



class System:
    def __init__(self):
        self.clients = [] 
        self.workers = [] 
        self.machines = [] 
        self.all_sum_workers = []
        self.all_sum_clients = []

        self.sum_machine_workers = [] 
        self.var_machine_workers = [] 
        self.avg_machine_workers = [] 
        self.sum_machine_clients= [] 
        self.var_machine_clients= [] 
        self.avg_machine_clients= [] 

    def calc_values(self):
        self.sum_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.sum_workers = [ 0 for x in xrange(len(self.workers[0]))]
        self.avg_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.avg_workers = [ 0 for x in xrange(len(self.workers[0]))]
        self.var_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.var_workers = [ 0 for x in xrange(len(self.workers[0]))]

        array_sum(len(self.clients), self.clients, self.sum_clients)
        array_sum(len(self.workers), self.workers, self.sum_workers)
        array_var(len(self.clients), self.clients, self.var_clients)
        array_var(len(self.workers), self.workers, self.var_workers)
        array_avg(len(self.clients), self.sum_clients, self.avg_clients, len(self.clients))
        array_avg(len(self.workers), self.sum_workers, self.avg_workers, len(self.workers))

        for i in xrange(len(self.machines)):
            self.all_sum_workers.append(self.machines[i].sum_workers)
            self.all_sum_clients.append(self.machines[i].sum_clients)
            
        self.sum_machine_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.sum_machine_workers = [ 0 for x in xrange(len(self.workers[0]))]
        self.avg_machine_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.avg_machine_workers = [ 0 for x in xrange(len(self.workers[0]))]
        self.var_machine_clients = [ 0 for x in xrange(len(self.clients[0]))]
        self.var_machine_workers = [ 0 for x in xrange(len(self.workers[0]))]

        array_sum(len(self.machines), self.all_sum_clients, self.sum_machine_clients)
        array_sum(len(self.machines), self.all_sum_workers, self.sum_machine_workers)
        array_var(len(self.machines), self.all_sum_clients, self.var_machine_clients)
        array_var(len(self.machines), self.all_sum_workers, self.var_machine_workers)
        array_avg(len(self.machines), self.sum_machine_workers, self.avg_machine_workers, len(self.machines))
        array_avg(len(self.machines), self.sum_machine_clients, self.avg_machine_clients, len(self.machines))




    def printAll(self):
        #printCSVFrom2DArray("clients",self.clients)   
        #printCSVFrom2DArray("workers",self.workers)   
        self.calc_values()
        print "comment:",
        printYellow("System Worker Stats")
        print "(1st attribute is replaced by # machines or workers)"
        printCsvLineFromArray("system_avg_machine_workers ", self.avg_machine_workers)
        printCsvLineFromArray("system_%var_machine_workers", self.var_machine_workers)
        printCsvLineFromArray("system_avg_workers         ", self.avg_workers)
        printCsvLineFromArray("system_%var_workers        ", self.var_workers)
        printCsvLineFromArray("system_sum_workers         ", self.sum_workers)
        print "comment:",
        printYellow("System Client Stats")
        print "(the 1st attribute is replaced # machines or clients)"
        printCsvLineFromArray("system_avg_machine_clients ", self.avg_machine_clients)
        printCsvLineFromArray("system_%var_machine_clients", self.var_machine_clients)
        printCsvLineFromArray("system_avg_clients         ", self.avg_clients)
        printCsvLineFromArray("system_%var_clients        ", self.var_clients)
        #printCsvLineFromArray("system_sum_machine_workers ", self.sum_machine_workers)
        printCsvLineFromArray("system_sum_clients         ", self.sum_clients)
        #printCsvLineFromArray("system_sum_machine_clients ", self.sum_machine_clients)



class Parser:
    def __init__(self):
        self.system = System()
        self.machines = []
        self.parseInputStats()
        self.printAll()

    def printAll(self):
        for i in xrange(len(self.machines)):
            print "comment:",
            printUnderlined("Machine "+str(i)+" Stats")
            print ""
            self.machines[i].printAll()
        self.system.printAll()
        
    def parseInputStats(self):
        for line in sys.stdin:                  # input from standard input
            if line[0] == '#':
                continue
            (command, words) = line.strip().split(":")
            command = command.strip()
            if command == 'comment' or command == 'file':
                continue
            elif command == 'worker':
                words = words.strip().split(",")
                self.machines[-1].workers.append([])
                self.system.workers.append([])
                for value in words:
                    self.machines[-1].workers[-1].append(float(value.strip()))
                    self.system.workers[-1].append(float(value.strip()))
            elif command == 'client':
                words = words.strip().split(",")
                self.machines[-1].clients.append([])
                self.system.clients.append([])
                for value in words:
                    self.machines[-1].clients[-1].append(float(value.strip()))
                    self.system.clients[-1].append(float(value.strip()))
            elif command == 'machine_id':
                self.machines.append(Machine(int(words.strip())))
        self.system.machines = self.machines
       

if __name__ == '__main__':
        Parser()
