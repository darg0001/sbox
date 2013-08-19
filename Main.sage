#!/usr/bin/env sage

r"""
    An example of how to use the class "Sbox" from external code

AUTHORS:

- Oleksandr Kazymyrov (2011): initial version

"""

#*****************************************************************************
#       Copyright (C) 2013 Oleksandr Kazymyrov <oleksandr.kazymyrov@ii.uib.no>
#
#  Distributed under the terms of the GNU General Public License (GPL)
#  as published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#                  http://www.gnu.org/licenses/
#***************************************************************************** 

os.chdir(os.path.split(os.path.abspath(sys.argv[0]))[0] + "/Sage")

load ./TestFunctions.sage

def main(argv=None):
    bits=6

    t1=cputime()

    #test_all_functions(bits=bits)
    #test_temp(bits=bits)
    test_equivalence(bits=bits)

    t2=cputime()

    print "====="
    print "Time = {0}".format(t2-t1)


if __name__ == "__main__":
    sys.exit(main())