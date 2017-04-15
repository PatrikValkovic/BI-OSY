from optparse import OptionParser

parser = OptionParser()
parser.add_option('-l', '--lines', help='Maximum count of lines for generated code', default=1000, type='int',
                  dest='countOfLines')
parser.add_option('-f', '--failChance', help='Percentage chance of disk fail', default=0.1, type='float',
                  dest='failChance')
parser.add_option('-r', '--readCheck', help='Check returned values from raid', default=True, dest='checkReturn')
parser.add_option('-d', '--devices', help='Count of devices', default=8, type='int', dest='countOfDevices')
parser.add_option('-s', '--sectors', help='Size of sectors', default=1 * 1024 * 2, type='int', dest='sizeOfSectors')

(options, args) = parser.parse_args()

failChance = options.failChance
countOfLines = options.countOfLines
checkReturn = options.checkReturn
countOfDevices = options.countOfDevices
sizeOfSectors = options.sizeOfSectors

print("File with maximum of " + str(countOfLines) + " will be generated")
print("Raid will have " + str(countOfDevices) + " devices, each with " + str(sizeOfSectors) + ' sectors')
print("Program " + ('will' if checkReturn else 'will not') + " control return values")
print("Disk have " + str(failChance) + "% to fail")
