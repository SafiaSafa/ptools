
"""extract_cmd - PTools extract command."""

from __future__ import print_function

import sys

import ptools


def create_subparser(parent):
    parser = parent.add_parser('extract', help=__doc__)
    parser.set_defaults(func=run)
    parser.add_argument('attract_output',
                        help='attract ouptut file')
    parser.add_argument('ligand',
                        help='ligand reduced topology')
    parser.add_argument('transid', type=int,
                        help='id of the translation to extract')
    parser.add_argument('rotid', type=int,
                        help='id of the rotation to extract')
    parser.add_argument('-o', '--output',
                        help='output topology file name (by default print '
                             'on stout)')


def rigidXMat44(rigid, mat):
    out = ptools.Rigidbody(rigid)
    for i in range(len(rigid)):
        coords = rigid.getCoords(i)
        coords2 = ptools.Coord3D()
        coords2.x = mat[0][0] * coords.x + mat[0][1] * coords.y + mat[0][2] * coords.z + mat[0][3]
        coords2.y = mat[1][0] * coords.x + mat[1][1] * coords.y + mat[1][2] * coords.z + mat[1][3]
        coords2.z = mat[2][0] * coords.x + mat[2][1] * coords.y + mat[2][2] * coords.z + mat[2][3]
        out.setCoords(i, coords2)
    return out


def run(args):
    ptools.io.check_file_exists(args.attract_output)
    ptools.io.check_file_exists(args.ligand)

    # Get transformation matrix from attract output file.
    docking_result = ptools.io.read_attract_output(args.attract_output)
    m = docking_result.get_matrix(args.transid, args.rotid)

    # Read ligand topology and apply transformation.
    ligand = ptools.Rigidbody(args.ligand)
    ligand_out = rigidXMat44(ligand, m)

    # Print output topology.
    header = 'REMARK 999 TRANSLATION ROTATION  {} {}'.format(args.transid,
                                                             args.rotid)
    if args.output:
        with open(args.output, 'wt') as f:
            print(header, ligand_out, sep='\n', file=f)
    else:
        print(header, ligand_out, sep='\n', file=sys.stdout)
