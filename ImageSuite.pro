TEMPLATE = subdirs

SUBDIRS += \
    SuiteCore \
    SuiteUI

SuiteUI.depends = SuiteCore
