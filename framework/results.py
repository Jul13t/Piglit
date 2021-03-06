# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

""" Module for results generation """

from __future__ import print_function, absolute_import

import collections
import copy

from framework import status, exceptions, grouptools

__all__ = [
    'TestrunResult',
    'TestResult',
]


class Subtests(collections.MutableMapping):
    """A dict-like object that stores Statuses as values."""
    def __init__(self, dict_=None):
        self.__container = {}

        if dict_ is not None:
            self.update(dict_)

    def __setitem__(self, name, value):
        self.__container[name] = status.status_lookup(value)

    def __getitem__(self, name):
        return self.__container[name]

    def __delitem__(self, name):
        del self.__container[name]

    def __iter__(self):
        return iter(self.__container)

    def __len__(self):
        return len(self.__container)

    def __repr__(self):
        return repr(self.__container)

    def to_json(self):
        res = dict(self)
        res['__type__'] = 'Subtests'
        return res

    @classmethod
    def from_dict(cls, dict_):
        if '__type__' in dict_:
            del dict_['__type__']

        res = cls(dict_)

        return res


class StringDescriptor(object):  # pylint: disable=too-few-public-methods
    """A Shared data descriptor class for TestResult.

    This provides a property that can be passed a str or unicode, but always
    returns a unicode object.

    """
    def __init__(self, name, default=unicode()):
        assert isinstance(default, unicode)
        self.__name = name
        self.__default = default

    def __get__(self, instance, cls):
        return getattr(instance, self.__name, self.__default)

    def __set__(self, instance, value):
        if isinstance(value, str):
            setattr(instance, self.__name, value.decode('utf-8', 'replace'))
        elif isinstance(value, unicode):
            setattr(instance, self.__name, value)
        else:
            raise TypeError('{} attribute must be a str or unicode instance, '
                            'but was {}.'.format(self.__name, type(value)))

    def __delete__(self, instance):
        raise NotImplementedError


class TestResult(object):
    """An object represting the result of a single test."""
    __slots__ = ['returncode', '_err', '_out', 'time', 'command', 'traceback',
                 'environment', 'subtests', 'dmesg', '__result', 'images',
                 'exception']
    err = StringDescriptor('_err')
    out = StringDescriptor('_out')

    def __init__(self, result=None):
        self.returncode = None
        self.time = float()
        self.command = str()
        self.environment = str()
        self.subtests = Subtests()
        self.dmesg = str()
        self.images = None
        self.traceback = None
        self.exception = None
        if result:
            self.result = result
        else:
            self.__result = status.NOTRUN

    @property
    def result(self):
        """Return the result of the test.

        If there are subtests return the "worst" value of those subtests. If
        there are not return the stored value of the test.

        """
        if self.subtests:
            return max(self.subtests.itervalues())
        return self.__result

    @result.setter
    def result(self, new):
        try:
            self.__result = status.status_lookup(new)
        except exceptions.PiglitInternalError as e:
            raise exceptions.PiglitFatalError(str(e))

    def to_json(self):
        """Return the TestResult as a json serializable object."""
        obj = {
            '__type__': 'TestResult',
            'command': self.command,
            'environment': self.environment,
            'err': self.err,
            'out': self.out,
            'result': self.result,
            'returncode': self.returncode,
            'subtests': self.subtests,
            'time': self.time,
            'exception': self.exception,
        }
        return obj

    @classmethod
    def from_dict(cls, dict_):
        """Load an already generated result in dictionary form.

        This is used as an alternate constructor which converts an existing
        dictionary into a TestResult object. It converts a key 'result' into a
        status.Status object

        """
        # pylint will say that assining to inst.out or inst.err is a non-slot
        # because self.err and self.out are descriptors, methods that act like
        # variables. Just silence pylint
        # pylint: disable=assigning-non-slot
        inst = cls()

        # TODO: There's probably a more clever way to do this
        for each in ['returncode', 'time', 'command', 'exception',
                     'environment', 'result', 'dmesg']:
            if each in dict_:
                setattr(inst, each, dict_[each])

        if 'subtests' in dict_:
            for name, value in dict_['subtests'].iteritems():
                inst.subtests[name] = value

        # out and err must be set manually to avoid replacing the setter
        if 'out' in dict_:
            inst.out = dict_['out']

        if 'err' in dict_:
            inst.err = dict_['err']

        return inst

    def update(self, dict_):
        """Update the results and subtests fields from a piglit test.

        Native piglit tests output their data as valid json, and piglit uses
        the json module to parse this data. This method consumes that raw
        dictionary data and updates itself.

        """
        if 'result' in dict_:
            self.result = dict_['result']
        elif 'subtest' in dict_:
            self.subtests.update(dict_['subtest'])


class Totals(dict):
    def __init__(self, *args, **kwargs):
        super(Totals, self).__init__(*args, **kwargs)
        for each in status.ALL:
            self[str(each)] = 0

    def __nonzero__(self):
        # Since totals are prepopulated, calling 'if not <Totals instance>'
        # will always result in True, this will cause it to return True only if
        # one of the values is not zero
        for each in self.itervalues():
            if each != 0:
                return True
        return False

    def to_json(self):
        """Convert totals to a json object."""
        result = copy.copy(self)
        result['__type__'] = 'Totals'
        return result

    @classmethod
    def from_dict(cls, dict_):
        """Convert a dictionary into a Totals object."""
        return cls(dict_)


class TestrunResult(object):
    """The result of a single piglit run."""
    def __init__(self):
        self.name = None
        self.uname = None
        self.options = None
        self.glxinfo = None
        self.wglinfo = None
        self.clinfo = None
        self.lspci = None
        self.time_elapsed = None
        self.tests = {}
        self.totals = collections.defaultdict(Totals)

    def calculate_group_totals(self):
        """Calculate the number of pases, fails, etc at each level."""
        for name, result in self.tests.iteritems():
            # If there are subtests treat the test as if it is a group instead
            # of a test.
            if result.subtests:
                for res in result.subtests.itervalues():
                    res = str(res)
                    temp = name

                    self.totals[temp][res] += 1
                    while temp:
                        temp = grouptools.groupname(temp)
                        self.totals[temp][res] += 1
                    self.totals['root'][res] += 1
            else:
                res = str(result.result)
                while name:
                    name = grouptools.groupname(name)
                    self.totals[name][res] += 1
                self.totals['root'][res] += 1

    def to_json(self):
        if not self.totals:
            self.calculate_group_totals()
        rep = copy.copy(self.__dict__)
        rep['__type__'] = 'TestrunResult'
        return rep

    @classmethod
    def from_dict(cls, dict_, _no_totals=False):
        """Convert a dictionary into a TestrunResult.

        This method is meant to be used for loading results from json or
        similar formats

        _no_totals is not meant to be used externally, it allows us to control
        the generation of totals when loading old results formats.

        """
        res = cls()
        for name in ['name', 'uname', 'options', 'glxinfo', 'wglinfo', 'lspci',
                     'tests', 'totals', 'results_version', 'clinfo']:
            value = dict_.get(name)
            if value:
                setattr(res, name, value)

        # This could be replaced with a getter/setter property
        time = dict_.get('time_elapsed')
        res.time_elapsed = None if time is None else float(time)

        if not res.totals and not _no_totals:
            res.calculate_group_totals()

        return res
