
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


class EvtReaderCallbacks(ctypes.Structure):
    pass

def EvtReaderCallbacks_New():
    c_New = conf.lib.OTF2_EvtReaderCallbacks_New
    c_New.argtypes = []
    c_New.restype = ctypes.POINTER(EvtReaderCallbacks)
    # NOTE: Do not errcheck here. This function returns a value, not an error code
    return c_New()

def EvtReaderCallbacks_Delete(evtReaderCallbacks):
    c_Delete = conf.lib.OTF2_EvtReaderCallbacks_Delete
    c_Delete.argtypes = [ ctypes.POINTER(EvtReaderCallbacks) ]
    c_Delete.restype = None
    return c_Delete(evtReaderCallbacks)

def EvtReaderCallbacks_Clear(evtReaderCallbacks):
    c_Clear = conf.lib.OTF2_EvtReaderCallbacks_Clear
    c_Clear.argtypes = [ ctypes.POINTER(EvtReaderCallbacks) ]
    c_Clear.restype = None
    return c_Clear(evtReaderCallbacks)

def _callback_wrapper(type, func, convert_args=None):
    def wrapper(location, time, eventPosition, userData, attribute_list, *args):
        if userData:
            py_userData = ctypes.cast(userData, ctypes.py_object).value
        else:
            py_userData = None
        try:
            if convert_args is not None:
                args = convert_args(*args)
            if not attribute_list:
                attribute_list = None
            ret = func(location, time, eventPosition, py_userData, attribute_list, *args)
            if ret is None:
                ret = CALLBACK_SUCCESS
        except:
            sys.stderr.write("An unhandled python exception has occurred in an "
                             "OTF2_EvtReaderCallback:\n")
            sys.stderr.write(traceback.format_exc())
            ret = CALLBACK_ERROR
        return ret.value
    return callback_wrapper(func, wrapper, type)

_EvtReaderCallback_FP_Unknown = ctypes.CFUNCTYPE(CallbackCode,
        LocationRef, TimeStamp, ctypes.c_uint64, ctypes.c_void_p,
        ctypes.POINTER(AttributeList))

@otf2 for event in events:
_EvtReaderCallback_FP_@@event.name@@ = ctypes.CFUNCTYPE(CallbackCode,
        LocationRef, TimeStamp, ctypes.c_uint64, ctypes.c_void_p,
        ctypes.POINTER(AttributeList)@@event.py_funcargtypes(argtypes=False)@@)

@otf2 endfor
def EvtReaderCallbacks_SetUnknownCallback(evtReaderCallbacks, unknownCallback):
    c_SetUnknownCallback = conf.lib.OTF2_EvtReaderCallbacks_SetUnknownCallback
    c_SetUnknownCallback.argtypes = [ ctypes.POINTER(EvtReaderCallbacks), _EvtReaderCallback_FP_Unknown ]
    c_SetUnknownCallback.restype = ErrorCode
    c_SetUnknownCallback.errcheck = HandleErrorCode
    wrapped_callback = _callback_wrapper(_EvtReaderCallback_FP_Unknown,
                                         unknownCallback)
    c_SetUnknownCallback(evtReaderCallbacks, wrapped_callback)

@otf2 for event in events:
@otf2  if event == Metric:
def EvtReaderCallbacks_SetMetricCallback(evtReaderCallbacks, metricCallback):
    c_SetMetricCallback = conf.lib.OTF2_EvtReaderCallbacks_SetMetricCallback
    c_SetMetricCallback.argtypes = [ ctypes.POINTER(EvtReaderCallbacks), _EvtReaderCallback_FP_Metric ]
    c_SetMetricCallback.restype = ErrorCode
    c_SetMetricCallback.errcheck = HandleErrorCode
    def convert_args_metric(metric, number_of_metrics, type_ids_array, metric_values_array):
        type_ids = [type_ids_array[i] for i in range(number_of_metrics)]
        metric_values = [metric_values_array[i] for i in range(number_of_metrics)]
        return metric, type_ids, metric_values
    wrapped_callback = _callback_wrapper(_EvtReaderCallback_FP_Metric,
                                         metricCallback, convert_args_metric)
    c_SetMetricCallback(evtReaderCallbacks, wrapped_callback)

@otf2  elif event == ProgramBegin:
def EvtReaderCallbacks_SetProgramBeginCallback(evtReaderCallbacks, programBeginCallback):
    c_SetProgramBeginCallback = conf.lib.OTF2_EvtReaderCallbacks_SetProgramBeginCallback
    c_SetProgramBeginCallback.argtypes = [ ctypes.POINTER(EvtReaderCallbacks), _EvtReaderCallback_FP_ProgramBegin ]
    c_SetProgramBeginCallback.restype = ErrorCode
    c_SetProgramBeginCallback.errcheck = HandleErrorCode
    def convert_args(program_name, number_of_arguments, arguments_array):
        arguments = [arguments_array[i] for i in range(number_of_arguments)]
        return program_name, arguments
    wrapped_callback = _callback_wrapper(_EvtReaderCallback_FP_ProgramBegin,
                                         programBeginCallback, convert_args)
    c_SetProgramBeginCallback(evtReaderCallbacks, wrapped_callback)

@otf2  else:
def EvtReaderCallbacks_Set@@event.name@@Callback(evtReaderCallbacks, @@event.lname@@Callback):
    c_Set@@event.name@@Callback = conf.lib.OTF2_EvtReaderCallbacks_Set@@event.name@@Callback
    c_Set@@event.name@@Callback.argtypes = [ ctypes.POINTER(EvtReaderCallbacks), _EvtReaderCallback_FP_@@event.name@@ ]
    c_Set@@event.name@@Callback.restype = ErrorCode
    c_Set@@event.name@@Callback.errcheck = HandleErrorCode
    wrapped_callback = _callback_wrapper(_EvtReaderCallback_FP_@@event.name@@,
                                         @@event.lname@@Callback)
    c_Set@@event.name@@Callback(evtReaderCallbacks, wrapped_callback)

@otf2  endif
@otf2 endfor
__all__ = [
    'EvtReaderCallbacks',
    'EvtReaderCallbacks_New',
    'EvtReaderCallbacks_Delete',
    'EvtReaderCallbacks_Clear',
    'EvtReaderCallbacks_SetUnknownCallback',
    @otf2 for event in events:
    'EvtReaderCallbacks_Set@@event.name@@Callback',
    @otf2 endfor
]
