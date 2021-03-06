
''' Generated by OTF2 Template Engine '''

import ctypes
import traceback
import sys

from .Config import conf, StrParam
from .ErrorCodes import ErrorCode, HandleErrorCode
from .GeneralDefinitions import *
from .AttributeList import AttributeList
from .Definitions import *
from .Events import *
from .Callbacks import callback_wrapper


class GlobalSnapReaderCallbacks(ctypes.Structure):
    pass

def GlobalSnapReaderCallbacks_New():
    c_New = conf.lib.OTF2_GlobalSnapReaderCallbacks_New
    c_New.argtypes = []
    c_New.restype = ctypes.POINTER(GlobalSnapReaderCallbacks)
    # NOTE: Do not errcheck here. This function returns a value, not an error code
    return c_New()

def GlobalSnapReaderCallbacks_Delete(globalSnapReaderCallbacks):
    c_Delete = conf.lib.OTF2_GlobalSnapReaderCallbacks_Delete
    c_Delete.argtypes = [ ctypes.POINTER(GlobalSnapReaderCallbacks) ]
    c_Delete.restype = None
    return c_Delete(globalSnapReaderCallbacks)

def GlobalSnapReaderCallbacks_Clear(globalSnapReaderCallbacks):
    c_Clear = conf.lib.OTF2_GlobalSnapReaderCallbacks_Clear
    c_Clear.argtypes = [ ctypes.POINTER(GlobalSnapReaderCallbacks) ]
    c_Clear.restype = None
    return c_Clear(globalSnapReaderCallbacks)

def _callback_wrapper(type, func, convert_args=None):
    def wrapper(location, time, userData, attribute_list, *args):
        if userData:
            py_userData = ctypes.cast(userData, ctypes.py_object).value
        else:
            py_userData = None
        try:
            if convert_args is not None:
                args = convert_args(*args)
            if not attribute_list:
                attribute_list = None
            ret = func(location, time, py_userData, attribute_list, *args)
            if ret is None:
                ret = CALLBACK_SUCCESS
        except:
            sys.stderr.write("An unhandled python exception has occurred in an "
                             "OTF2_GlobalSnapReaderCallback:\n")
            sys.stderr.write(traceback.format_exc())
            ret = CALLBACK_ERROR
        return ret.value
    return callback_wrapper(func, wrapper, type)

_GlobalSnapReaderCallback_FP_Unknown = ctypes.CFUNCTYPE(CallbackCode,
        LocationRef, TimeStamp, ctypes.c_void_p,
        ctypes.POINTER(AttributeList))

@otf2 for snap in snaps:
_GlobalSnapReaderCallback_FP_@@snap.name@@ = ctypes.CFUNCTYPE(CallbackCode,
        LocationRef, TimeStamp, ctypes.c_void_p,
        ctypes.POINTER(AttributeList)@@snap.py_funcargtypes(argtypes=False)@@)

@otf2 endfor
def GlobalSnapReaderCallbacks_SetUnknownCallback(globalSnapReaderCallbacks, unknownCallback):
    c_SetUnknownCallback = conf.lib.OTF2_GlobalSnapReaderCallback_SetUnknownCallback
    c_SetUnknownCallback.argtypes = [ ctypes.POINTER(GlobalSnapReaderCallbacks), _GlobalSnapReaderCallback_FP_Unknown ]
    c_SetUnknownCallback.restype = ErrorCode
    c_SetUnknownCallback.errcheck = HandleErrorCode
    wrapped_callback = _callback_wrapper(_GlobalSnapReaderCallback_FP_Unknown,
                                         unknownCallback)
    c_SetUnknownCallback(globalSnapReaderCallbacks, wrapped_callback)

@otf2 for snap in snaps:
@otf2  if snap == MetricSnap:
def GlobalSnapReaderCallbacks_SetMetricCallback(globalSnapReaderCallbacks, metricCallback):
    c_SetMetricCallback = conf.lib.OTF2_GlobalSnapReaderCallbacks_SetMetricCallback
    c_SetMetricCallback.argtypes = [ ctypes.POINTER(GlobalSnapReaderCallbacks), _GlobalSnapReaderCallback_FP_Metric ]
    c_SetMetricCallback.restype = ErrorCode
    c_SetMetricCallback.errcheck = HandleErrorCode
    def convert_args_metric(metric, number_of_metrics, type_ids_array, metric_values_array):
        type_ids = [type_ids_array[i] for i in range(number_of_metrics)]
        metric_values = [metric_values_array[i] for i in range(number_of_metrics)]
        return metric, type_ids, metric_values
    wrapped_callback = _callback_wrapper(_GlobalSnapReaderCallback_FP_Metric,
                                         metricCallback, convert_args_metric)
    c_SetMetricCallback(globalSnapReaderCallbacks, wrapped_callback)

@otf2  else:
def GlobalSnapReaderCallbacks_Set@@snap.name@@Callback(globalSnapReaderCallbacks, @@snap.lname@@Callback):
    c_Set@@snap.name@@Callback = conf.lib.OTF2_GlobalSnapReaderCallbacks_Set@@snap.name@@Callback
    c_Set@@snap.name@@Callback.argtypes = [ ctypes.POINTER(GlobalSnapReaderCallbacks), _GlobalSnapReaderCallback_FP_@@snap.name@@ ]
    c_Set@@snap.name@@Callback.restype = ErrorCode
    c_Set@@snap.name@@Callback.errcheck = HandleErrorCode
    wrapped_callback = _callback_wrapper(_GlobalSnapReaderCallback_FP_@@snap.lname@@Callback,
                                         @@snap.lname@@Callback)
    c_Set@@snap.name@@Callback(globalSnapReaderCallbacks, wrapped_callback)

@otf2  endif
@otf2 endfor
__all__ = [
    'GlobalSnapReaderCallbacks',
    'GlobalSnapReaderCallbacks_New',
    'GlobalSnapReaderCallbacks_Delete',
    'GlobalSnapReaderCallbacks_Clear',
    'GlobalSnapReaderCallbacks_SetUnknownCallback',
    @otf2 for snap in snaps:
    'GlobalSnapReaderCallbacks_Set@@snap.name@@Callback',
    @otf2 endfor
]
