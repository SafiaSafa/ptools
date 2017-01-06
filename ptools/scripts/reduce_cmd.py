
"""PTools reduce command."""

from __future__ import print_function

import copy
import itertools
import os
import sys

import yaml

import ptools


DEFAULT_PROT_REDUCTION_DATA = os.path.join(ptools.DATA_DIR, 'at2cg.prot.dat')
DEFAULT_DNA_REDUCTION_DATA = os.path.join(ptools.DATA_DIR, 'at2cg.dna.dat')
DEFAULT_FF_PARAM_DATA = os.path.join(ptools.DATA_DIR, 'ff_param.dat')
DEFAULT_CONVERSION_DATA = os.path.join(ptools.DATA_DIR, 'type_conversion.dat')

DEFAULT_ATTRACT2_REDUCTION_DATA = os.path.join(ptools.DATA_DIR, 'at2cg_attract2.yml')


class AtomInBead:
    """Class definition for an atom in a bead"""
    def __init__(self, name, wgt):
        self.name = name    # atom name
        self.x = 0.0        # x coordinate
        self.y = 0.0        # y coordiante
        self.z = 0.0        # z coordinate
        self.weight = wgt   # atom weight (within a bead)
        self.found = 0      # atom found or not (not found by default)

    def Show(self):
        return "%s %8.3f %8.3f %8.3f %3.1f %d" % (self.name, self.x, self.y, self.z, self.weight, self.found)


class BeadAttract1:
    """Class definition for a bead"""
    def __init__(self, name, id):
        self.name = name        # bead name
        self.id = id            # bead id
        self.size = 0           # bead size (number of atoms inside)
        self.listOfAtomNames = []  # list of all atom names
        self.listOfAtoms = []  # list of all atoms (AtomInBead)

    def Show(self):
        return "%s %d %d" % (self.name, self.id, self.size)


class CoarseRes:
    """Class definition for coarse grain (reduced) protein residue (or DNA base)"""
    def __init__(self):
        self.listOfBeadId = []      # list of bead id in res
        self.listOfBeads = []       # list of beads in res

    def __repr__(self):
        name = self.__class__.__name__
        nbeads = len(self.listOfBeads)
        return '{}({} beads)'.format(name, nbeads)

    def Add(self, residue):
        """Add in a residue atoms from bead"""
        for at in residue:
            at_name = at[1]
            at_wgt = float(at[2])
            bd_id = int(at[3])
            bd_name = at[4]
            if at_name != 'EMPTY':  # EMPTY is a special tag to deal with glycine
                # in bead not in residue than create it
                if bd_id not in self.listOfBeadId:
                    self.listOfBeadId.append(bd_id)
                    self.listOfBeads.append(BeadAttract1(bd_name, bd_id))
                # add atom in bead in residue
                bead_position = self.listOfBeadId.index(bd_id)
                bead = self.listOfBeads[bead_position]
                bead.listOfAtomNames.append(at_name)
                atInBd = AtomInBead(at_name, at_wgt)
                bead.listOfAtoms.append(atInBd)
                bead.size += 1
                # update bead in residue
                self.listOfBeads[bead_position] = bead
        # return the number of bead per residue
        return len(self.listOfBeadId)

    def FillAtom(self, at_name, x, y, z):
        """Fill an atom from bead with coordinates"""
        # quickly check atom in atom list
        # 1: browse beads
        for bead in self.listOfBeads:
            # 2: browse atoms in bead
            if at_name in bead.listOfAtomNames:
                # then find exactly where this atom is present
                for atom in bead.listOfAtoms:
                    if at_name == atom.name:
                        atom.x = x
                        atom.y = y
                        atom.z = z
                        atom.found = 1

    def Reduce(self, infoResName, infoResId, fail_on_error=True):
        """Reduce a bead with atoms present in bead"""
        output = []
        # reduce all beads in a residue
        # for each bead in the residue
        for bead in self.listOfBeads:
            reduce_size = 0
            reduce_x = 0.0
            reduce_y = 0.0
            reduce_z = 0.0
            sum_wgt = 0.0
            # for each atom of a bead
            for atom in bead.listOfAtoms:
                if atom.found == 1:
                    reduce_size += 1
                    reduce_x += atom.x * atom.weight
                    reduce_y += atom.y * atom.weight
                    reduce_z += atom.z * atom.weight
                    sum_wgt += atom.weight
                else:
                    message = "missing atom %(atomname)s "\
                              "in bead %(beadname)s %(beadid)2d "\
                              "for residue %(resname)s:%(resid)d. "\
                              "Please fix your PDB!\n" % {'atomname': atom.name,
                                                          'beadname': bead.name,
                                                          'beadid': bead.id,
                                                          'resname': infoResName,
                                                          'resid': infoResId}
                    if fail_on_error:
                        raise Exception(message)
                    else:
                        ptools.io.warning(message)
                        ptools.io.warning("Continue execution as required...")
            if reduce_size == bead.size:
                x = reduce_x / sum_wgt
                y = reduce_y / sum_wgt
                z = reduce_z / sum_wgt
                coord = ptools.Coord3D(x, y, z)
                output.append([coord, bead.name, bead.id])
        return output

    def Show(self):
        for bead in self.listOfBeads:
            print(bead.name, bead.id, bead.size, bead.atomIdList)


def create_attract1_subparser(parent):
    parser = parent.add_parser('attract1',
                               help='reduce using the attract1 force field')
    parser.set_defaults(forcefield='attract1')

    group = parser.add_mutually_exclusive_group()
    group.add_argument('--prot', action='store_true', dest='molProt',
                       help='reduce protein')
    group.add_argument('--dna', action='store_true', dest='molDNA',
                       help='reduce DNA')

    parser.add_argument('--red', dest='redName',
                        help="path to correspondance file between atoms and beads file")
    parser.add_argument('--ff', dest='ffName',
                        default=DEFAULT_FF_PARAM_DATA,
                        help="path to force field parameter file")
    parser.add_argument('--conv', dest='convName',
                        default=DEFAULT_CONVERSION_DATA,
                        help="path type conversion file")
    parser.add_argument('--allow_missing', action='store_true',
                        dest='warning',
                        help="don't stop program if atoms are missing, "
                             "only display a warning on stderr")


def create_attract2_subparser(parent):
    parser = parent.add_parser('attract2',
                               help='reduce using the attract2 force field')
    parser.set_defaults(forcefield='attract2')
    parser.add_argument('--red', dest='redName',
                        help="path to correspondance file between atoms and beads file")
    parser.add_argument('--allow_missing', action='store_true',
                        dest='warning',
                        help="don't stop program if atoms are missing, "
                             "only display a warning on stderr")


def create_subparser(parent):
    parser = parent.add_parser('reduce', help=__doc__)
    parser.set_defaults(func=run)

    parser.add_argument('pdb',
                        help="path to input PDB file (atomistic resolution)")

    subparsers = parser.add_subparsers()
    create_attract1_subparser(subparsers)
    create_attract2_subparser(subparsers)


def get_reduction_data_path(args):
    """Return path to reduction data file.

    Reduction data file can be provided from the '--red' option.

    Returns:
        str: path to reduction parameter file.
    """
    if not args.redName:
        if args.forcefield == 'attract1':
            if args.molProt:
                return DEFAULT_PROT_REDUCTION_DATA
            elif args.molDNA:
                return DEFAULT_DNA_REDUCTION_DATA
            else:
                err = "error: one of the arguments --prot --dna is required when "\
                      "not using --red option"
                print(err, file=sys.stderr)
                sys.exit(2)
        elif args.forcefield == 'attract2':
            return DEFAULT_ATTRACT2_REDUCTION_DATA
    return args.redName


def atomtag(resname, atomname):
    """Return a string in the format <residue_name>-<atom_name>."""
    return '{}-{}'.format(resname, atomname)


def residuetag(resname, resid, chain):
    """Return a string in the format <residue_name>-<residue_id>-<chain_id>."""
    return '{}-{}-{}'.format(resname, resid, chain)


def read_reduction_parameters(path):
    """Read file that contains parameters for correspondance between atoms
    and beads.

    Args:
        path (str): path to parameter file.

    Returns:
        dict[str]->CoarseRes: a dictionary mapping a residue name with a
            CoarseRes instance.
    """

    # Read file and store non comment lines into a list of tokens.
    allitems = []
    with open(path, 'rt') as f:
        for lineid, line in enumerate(f):
            if not ptools.io.is_comment(line):
                items = line.split()

                # Dies if less that 5 columns on the line.
                if len(items) < 5:
                    msg = 'expected at least 5 items (found {})'.format(len(items))
                    raise ptools.io.FileParsingError(path, msg, line, lineid + 1)

                allitems.append(items[:5])

    # Sort items in reverse order ensure that '*' lines are at the start of
    # the list (and also is sorting is required for itertools.groupby)
    allitems.sort(key=lambda items: items[0], reverse=False)

    # Construct the output structure in two steps.

    # 1. Create empty coarse residues.
    resBeadAtomModel = {}
    for res, residue in itertools.groupby(allitems, lambda items: items[0]):
        if res != '*':
            resBeadAtomModel[res] = CoarseRes()

    # 2. Add beads to residues.
    for res, residue in itertools.groupby(allitems, lambda items: items[0]):
        residue = list(residue)
        if res == '*':
            # Atoms named '*' have to be added to all residues.
            for bead in resBeadAtomModel.values():
                bead.Add(residue)
        else:
            resBeadAtomModel[res].Add(residue)

    # NOTE: the construction could be done in a single step.
    # It is done this way because doing it in a single step would change
    # beads order in output file (see diff with ad64a4a).

    return resBeadAtomModel


def read_forcefield_parameters(path):
    """Read force field parameter file.

    Args:
        path (str): path to parameter file.

    Returns:
        dict[int]->float: dictionary mapping the bead id with its charge.
    """
    charge_dict = {}
    with open(path, 'rt') as f:
        for lineid, line in enumerate(f):
            if not ptools.io.is_comment(line):
                items = line.split()
                # Dies if less that 5 columns on the line.
                if len(items) < 5:
                    msg = 'expected at least 5 items (found {})'.format(len(items))
                    raise ptools.io.FileParsingError(path, msg, line, lineid + 1)
                bead_id = int(items[0])
                bead_charge = float(items[3])
                charge_dict[bead_id] = bead_charge
    return charge_dict


def read_type_conversion_parameters(path):
    """Read atom and residue type conversion parameter file.

    Args:
        path (str): path to parameter file.

    Returns:
        dict[str]->str: dictionary mapping old residue names with new ones
        dict[str]->str: dictionary mapping old atom names with new ones
    """
    def raise_invalid_number_of_tokens(path, tokens, line, lineid):
        msg = 'expected 2 tokens for residue type conversion and '\
              '3 tokens for atom typeconversion '\
              '(found {})'.format(len(tokens))
        raise ptools.io.FileParsingError(path, msg, line, lineid)

    def warn_duplicate_entry(entry, lineid):
        msg = 'duplicate entry {} at line {}'.format(entry, lineid)
        ptools.io.warning(msg)

    def parse_residue_conversion():
        res_old, res_new = items
        if res_old in res_conv:
            warn_duplicate_entry(res_old, lineid + 1)
        else:
            res_conv[res_old] = res_new

    def parse_atom_conversion():
        res, atom_old, atom_new = items
        entry_old = atomtag(res, atom_old)
        entry_new = atomtag(res, atom_new)
        if entry_old in atom_conv:
            warn_duplicate_entry(entry_old, lineid + 1)
        else:
            atom_conv[entry_old] = entry_new

    res_conv = {}
    atom_conv = {}
    with open(path, 'rt') as f:
        for lineid, line in enumerate(f):
            if not ptools.io.is_comment(line):
                items = line.split()
                if len(items) == 2:
                    parse_residue_conversion()
                elif len(items) == 3:
                    parse_atom_conversion()
                else:
                    raise_invalid_number_of_tokens(path, items, line, lineid + 1)

    return res_conv, atom_conv


def read_atomic(path, res_conv, atom_conv):
    """Read all atom topology file.

    Convert residue and atom names on the fly.

    Args:
        path (str): path to topology file.
        res_conv (dict[str]->str): map old residue names with new ones.
        atom_conv (dict[str]->str): map old atom names with new ones

    Returns:
        list[]: atoms read from input file.
    """
    rb = ptools.Rigidbody(path)
    atomlist = []
    for i in xrange(len(rb)):
        atom = rb.CopyAtom(i)

        # Residue name conversion.
        resname = atom.residType
        if resname in res_conv:
            atom.residType = res_conv[resname]

        # Atom name conversion.
        atomname = atomtag(atom.residType, atom.atomType)
        if atomname in atom_conv:
            name = atom_conv[atomname].split('-')[1]
            atom.atomType = name

        atomlist.append(atom)
    return atomlist


def count_residues(atomlist, residue_to_cg):
    """Create the list of residue tags and residue beads.

    Args:
        atomlist (list[ptools.Atom]): list of all atoms read from topology.
        residue_to_cg (dict[str]->CoarseRes): map the residue name with a
            coarse grain representation.

    Returns:
        list[str]: list of residue tags (one per residue).
        list[CoarseRes]: list of beads (one per residue).
    """
    restaglist = []
    beadlist = []
    for atom in atomlist:
        resname = atom.residType
        restag = residuetag(resname, atom.residId, atom.chainId)
        if restag not in restaglist:
            if resname in residue_to_cg:
                restaglist.append(restag)
                beadlist.append(copy.deepcopy(residue_to_cg[resname]))
            else:
                msg = 'residue {} is unknown the residues <-> beads <-> atoms '\
                      'list!! It will not be reduced into coarse grain'.format(resname)
                ptools.io.warning(msg)
    return restaglist, beadlist


def fill_beads(atomlist, restaglist, beadlist):
    """
    Args:
        atomlist (list[ptools.Atom]): list of all atoms read from topology.
        restaglist (list[str]): list of residue tags (one per residue).
        beadlist (list[CoarseRes]): list of beads (one per residue).
    """
    for atom in atomlist:
        restag = residuetag(atom.residType, atom.residId, atom.chainId)
        if restag in restaglist:
            index = restaglist.index(restag)
            beadlist[index].FillAtom(atom.atomType,
                                     atom.coords.x,
                                     atom.coords.y,
                                     atom.coords.z)


def reduce_beads(restaglist, beadlist, bead_charge_map):
    """Reduction to coarse grain model.

    Args:
        restaglist (list[str]): list of residue tags (one per residue).
        beadlist (list[CoarseRes]): list of beads (one per residue).
        bead_charge_map (dict[int]->float): dictionary mapping the bead id with
            its charge.
        keep_original_order (bool): sort beads as in older version of PTools.

    Returns:
        list[ptools.Atom]: coarse grain model as an atom list.
    """
    cgmodel = []
    atom_count = 0
    for i, restag in enumerate(restaglist):
        tag = restag.split('-')
        resname = tag[0]
        resid = int(tag[1])
        resbead = beadlist[i].Reduce(resname, resid)
        for bead in resbead:
            coord = bead[0]
            atomname = bead[1]
            atomtypeid = bead[2]
            atomcharge = 0.0
            if atomtypeid in bead_charge_map:
                atomcharge = bead_charge_map[atomtypeid]
            else:
                msg = "cannot find charge for bead {} {:2d}... defaults to 0.0"
                ptools.io.warning(msg.format(atomname, atomtypeid))
            atom_count += 1

            prop = ptools.Atomproperty()
            prop.atomType = atomname
            prop.atomId = atom_count
            prop.residId = resid
            prop.residType = resname
            prop.chainId = ' '
            prop.extra = '{:5d}{:8.3f}{:2d}{:2d}'.format(atomtypeid,
                                                         atomcharge, 0, 0)
            newatom = ptools.Atom(prop, coord)

            cgmodel.append(newatom)
    return cgmodel


def print_red_output(cgmodel, forcefield):
    """Print coarse grain model to stdout as a reduced PDB file.

    Args:
        cgmodel (list[ptools.Atom]): coarse grain atom list.
    """
    print("HEADER    {} REDUCED PDB FILE".format(forcefield))
    print('\n'.join(atom.ToPdbString() for atom in cgmodel))


def run(args):
    fmap = {'attract1': run_attract1,
            'attract2': run_attract2}
    fmap[args.forcefield](args)


def run_attract1(args):
    redname = get_reduction_data_path(args)
    ffname = args.ffName
    convname = args.convName
    atomicname = args.pdb

    ptools.io.check_file_exists(redname)
    ptools.io.check_file_exists(ffname)
    ptools.io.check_file_exists(convname)
    ptools.io.check_file_exists(atomicname)

    resBeadAtomModel = read_reduction_parameters(redname)
    beadChargeDict = read_forcefield_parameters(ffname)
    resConv, atomConv = read_type_conversion_parameters(convname)

    atomList = read_atomic(atomicname, resConv, atomConv)
    residueTagList, coarseResList = count_residues(atomList, resBeadAtomModel)
    fill_beads(atomList, residueTagList, coarseResList)
    cgmodel = reduce_beads(residueTagList, coarseResList, beadChargeDict)
    print_red_output(cgmodel)


# --------------------------------------------------------------------------
# Attract 2 material
# --------------------------------------------------------------------------


class Bead(ptools.Atomproperty):
    def __init__(self, atoms, parameters):
        self.atoms = atoms
        self.atomType = parameters.get('name', 'X')
        self.atomCharge = parameters.get('charge', 0.0)
        self.chainId = self.atoms[0].chainId

        typeid = parameters.get('typeid', 0)
        self.extra = '{:5d}{:8.3f} 0 0'.format(typeid, self.atomCharge)

        # List of atom names that should be part of the bead.
        self.atom_names = parameters['atoms']

    @property
    def coords(self):
        """Bead coordinates, i.e. the center of mass of the atoms
        constituting the bead.
        """
        x = ptools.Coord3D()
        for atom in self.atoms:
            x += atom.coords
        n = 1. / len(self.atoms)
        return x * n

    def toAtom(self):
        """Return a ptools.Atom instance with current bead properties and
        coordinates."""
        return ptools.Atom(self, self.coords)

    def topdb(self):
        return self.toAtom().ToPdbString()


class CoarseResidue:
    """Create a residue coarse grain model from an atomistic model."""
    def __init__(self, resname, resid, resatoms, parameters):
        self.resname = resname
        self._resid = resid
        self.beads = []

        for bead_param in parameters:
            atoms = [a for a in resatoms if a.atomType in bead_param['atoms']]
            rc = self._compare_expected_and_found_atoms(bead_param, atoms)
            if rc != 2:
                b = Bead(atoms, bead_param)
                b.residType = self.resname
                b.residId = self.resid
                self.beads.append(b)
            else:
                msg = "skipping residue {}:{}".format(resname, resid)
                ptools.io.warning(msg)

    def _compare_expected_and_found_atoms(self, bead_param, atoms):
        """Compare expected atoms from bead parameters to atoms actually found
        in the residue.

        Returns:
            int: 0 if expected atoms are the same as found atoms
                 1 if they differ
                 2 if no expected atom has been found
        """
        if not atoms:
            err = 'no atom found for bead {} (atoms={}) of residue {}:{}'
            err = err.format(bead_param['name'], bead_param['atoms'],
                             self.resname, self.resid)
            ptools.io.warning(err)
            return 2
        elif len(atoms) != len(bead_param['atoms']):
            msg = 'residue %(resname)s:%(resid)d, bead %(bead_name)s: '\
                  'expected atoms %(expected_atoms)s, '\
                  'found %(found_atoms)s' % {
                      'resname': self.resname,
                      'resid': self.resid,
                      'bead_name': bead_param['name'],
                      'expected_atoms': sorted(bead_param['atoms']),
                      'found_atoms': sorted(a.atomType for a in atoms)
                  }
            ptools.io.warning(msg)
            return 1
        return 0

    @property
    def resid(self):
        return self._resid

    @resid.setter
    def resid(self, value):
        for b in self.beads:
            b.residId = value
        self._resid = value

    def topdb(self):
        return '\n'.join(b.topdb() for b in self.beads)


class Reducer:
    """Class that handle reduction from an atomistic topology to a coarse
    grain model.

    The reduction consists of two steps, namely preprocessing and reduction
    itself.

    1 - Preprocessing
    -----------------

    Atom and residues from the all-atom topology are renamed if
    necessary to make sure they match the reduction parameter file.

    As an example, it is quite common that C-terminal oxygen atoms are named
    'OT' (or any variation). Some coarse grain models such as ATTRACT1 and
    ATTRACT2 will consider those atom in the exact same way as other oxygen
    atoms. Hence, those models require renaming OT atoms into 'O'.

    2 - Reduction
    -------------

    - top <- read all atom topology
    - foreach residue in top
          check residue is defined is reduction parameters
          CREATE_RESIDUE_BEADS(residue, reduction parameters)

    PROCEDURE CREATE_RESIDUE_BEADS (residue, parameters)
        foreach bead_parameter in parameters
            atoms <- find atoms in residu that belong to current bead
            check number of atoms found vs number that should have been found
            create a bead:
                name, type, charge, etc. are determined from parameters
                coordinates is the barycentre of atoms that belong to the bead


    Attributes:
        residue_rename (dict[str]->str): map source residue name with target
            residue name (see Preprocessing).
        atom_rename (dict[str]->dict[str]->str): map target residue name
            with map mapping source atom name with target atom name
            (see Preprocessing).
    """

    residue_rename = {}
    atom_rename = {}

    def __init__(self, topology_file, reduction_parameters_file):
        """Initialize Reduce from topology file and reduction parameter file.

        Args:
            topology_file (str): path to all-atom topology file (PDB format).
            reduction_parameter_file (str): path to reduction parameter file
                (YAML format).

        Attributes:
            allatom_file (str): path to all-atom topology file
            reduction_file (str): path to reduction parameter file
            atoms (list[ptools.Atom]): list of all atoms read from topology
            forcefield (str): force field name read from reduction parameter
                file
            reduction_parameters (dict[str]->list): map residue names with a
                list of bead parameter for each bead in a residue.
            beads (list[Bead]): list all coarse grain beads for this model
        """
        self.allatom_file = topology_file
        self.reduction_file = reduction_parameters_file
        self.atoms = []
        self.forcefield = ''
        self.reduction_parameters = {}
        self.beads = []

        ptools.io.check_file_exists(self.reduction_file)
        ptools.io.check_file_exists(self.allatom_file)

        self.read_reduction_parameters()
        self.read_topology()

    def read_reduction_parameters(self):
        """Read YAML reduction parameter file."""
        with open(self.reduction_file, 'rt') as f:
            data = yaml.load(f)
        self.forcefield = data['forcefield']
        self.reduction_parameters = data['beads']

    def read_topology(self):
        """Read PDB topology file."""
        rb = ptools.Rigidbody(self.allatom_file)
        self.atoms = [rb.CopyAtom(i) for i in xrange(len(rb))]

    def rename_atoms_and_residues(self):
        """Rename atom and residues according to data in rename maps."""
        def should_rename_residue():
            return atom.residType in self.residue_rename

        def rename_residue():
            atom.residType = self.residue_rename[atom.residType]

        def should_rename_atom_for_every_residue():
            """If '*' is in the atom rename map, all residues are affected."""
            return '*' in self.atom_rename and atom.atomType in self.atom_rename['*']

        def should_rename_atom():
            return atom.residType in self.atom_rename and \
                atom.atomType in self.atom_rename[atom.residType]

        def rename_atom(name):
            atom.atomType = name

        for atom in self.atoms:
            if should_rename_residue():
                rename_residue()

            if should_rename_atom_for_every_residue():
                rename_atom(self.atom_rename['*'][atom.atomType])

            if should_rename_atom():
                rename_atom(self.atom_rename[atom.residType][atom.atomType])

    def reduce(self):
        """Actual reduction method.

        Group atoms by residue then iterate over those residues to create
        coarse grain residues.
        """
        def has_rule_for_residue_reduction():
            if resname not in self.reduction_parameters:
                msg = "don't know how to handle residue {0} "\
                      "(no reduction rule found for this residue)..."\
                      "skipping this residue".format(resname)
                ptools.io.warning(msg)
                return False
            return True

        # Rename atoms and residues so that they will match reduction
        # parameters.
        self.rename_atoms_and_residues()

        # Residue list: group atoms by residue tag.
        # A residue is two items: (<residue tag>, <atom list iterator>).
        residue_list = itertools.groupby(self.atoms,
                                         key=lambda atom: residuetag(atom.residType,
                                                                     atom.chainId,
                                                                     atom.residId))
        atomid = 1
        for restag, resatoms in residue_list:
            resname, chain, resid = restag.split('-')

            if has_rule_for_residue_reduction():
                coarse_res = CoarseResidue(resname, int(resid),
                                           list(resatoms),
                                           self.reduction_parameters[resname])
                # Update bead atom id.
                for bead in coarse_res.beads:
                    bead.atomId = atomid
                    atomid += 1

                self.beads += coarse_res.beads

    def print_output_model(self):
        """Print coarse grain model in reduced PDB format."""
        forcefield = self.forcefield
        header = 'HEADER    {} REDUCED PDB FILE'.format(forcefield)
        content = '\n'.join(bead.toAtom().ToPdbString() for bead in self.beads)
        print(header, content, sep='\n')


def run_attract2(args):
    redname = get_reduction_data_path(args)
    topology = args.pdb

    reducer = Reducer(topology, redname)
    reducer.residue_rename = {'HIE': 'HIS', 'LEU': 'LEU'}
    reducer.atom_rename = {'*': {'OT': 'O', 'OT1': 'O', 'OT2': 'O'},
                           'ILE': {'CD': 'CD1'}}

    reducer.reduce()
    reducer.print_output_model()
