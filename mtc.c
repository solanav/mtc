#define PY_SSIZE_T_CLEAN
#include <python3.7/Python.h>
#include <pthread.h>

#define MOD_NAME "mtc"

typedef struct _thread_args
{
    PyObject *function;
    PyObject *list;
    PyThreadState *self_ts;
} thread_args;

void *thread_subroutine(void *input)
{
    thread_args *args = (thread_args *)input;

    Py_ssize_t list_size = PyList_Size(args->list);
    PyObject *list_item;
    for (int i = 0; i < list_size; i++)
    {
        list_item = PyList_GetItem(args->list, i);

        PyEval_AcquireThread(args->self_ts);
        PyEval_CallFunction(args->function, "O", list_item);
        PyEval_ReleaseThread(args->self_ts);
    }

    return NULL;
}

static PyObject *launch_thread(PyObject *self, PyObject *args)
{
    PyObject *function;
    PyObject *list_list;

    if (!PyArg_ParseTuple(args, "OO", &function, &list_list))
    {
        printf("[ERROR] Error parsing arguments\n");
        return NULL;
    }

    // Check arguments
    if (!PyList_Check(list_list))
    {
        printf("[ERROR] That is not a list\n");
        return NULL;
    }

    if (!PyCallable_Check(function))
    {
        printf("[ERROR] Object is not callable\n");
        return NULL;
    }

    PyThreadState *main_ts = NULL;
    main_ts = PyThreadState_Get();
    PyInterpreterState *main_is = main_ts->interp;
    PyEval_ReleaseThread(main_ts);

    // This is also the number of threads to launch
    Py_ssize_t list_size = PyList_Size(list_list);

    pthread_t *thread_list = (pthread_t *)calloc(list_size, sizeof(pthread_t));
    for (Py_ssize_t i = 0; i < list_size; i++)
    {
        PyObject *list = PyList_GetItem(list_list, i);

        if (!PyList_Check(list_list))
        {
            printf("[ERROR] Item number %ld inside list is not a list\n", i);
            return NULL;
        }

        PyEval_AcquireThread(main_ts);
        PyThreadState *self_ts = PyThreadState_New(main_is);
        PyEval_ReleaseThread(main_ts);

        // Pack all the arguments
        thread_args args;
        args.function = function;
        args.list = list;
        args.self_ts = self_ts;

        int res = pthread_create(&(thread_list[i]), NULL, thread_subroutine, (void *)&args);
        if (res)
        {
            printf("[ERROR] Error starting thread number %ld\n", i);
            return NULL;
        }

        printf("[START] Thread number %ld\n", i);
    }

    for (Py_ssize_t i = 0; i < list_size; i++)
    {
        pthread_join(thread_list[i], NULL);
        printf("[END] Thread number %ld\n", i);
    }

    free(thread_list);

    Py_RETURN_NONE;
}

static PyMethodDef MTCMethods[] = {
    {"launch", launch_thread, METH_VARARGS, "Launch a thread that executes the given function"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef mtcmodule = {
    PyModuleDef_HEAD_INIT,
    MOD_NAME,
    NULL,
    -1,
    MTCMethods};

PyMODINIT_FUNC PyInit_mtc(void)
{
    return PyModule_Create(&mtcmodule);
}

int main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL)
    {
        printf("[ERROR] Cannot decode locale\n");
        return 1;
    }

    PyImport_AppendInittab(MOD_NAME, PyInit_mtc);

    Py_SetProgramName(program);

    Py_Initialize();

    PyEval_InitThreads();

    PyImport_ImportModule(MOD_NAME);

    PyMem_RawFree(program);

    return 0;
}