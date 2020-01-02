package com.suning.aps.mtai.api;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.Arrays;
import java.util.List;

public interface MTaiLib extends Library {
    public static final String JNA_LIBRARY_NAME = "mtai"; //"libmtai.so";
    public static final JnaTools INSTANCE = (JnaTools) Native.loadLibrary(JnaTools.JNA_LIBRARY_NAME, JnaTools.class);
    /*public static class _Person extends Structure {
    public int age;
    public byte[] name = new byte[11];
    public byte[] type = new byte[20];
    public _Person() {
        super();
    }
    protected List<String> getFieldOrder() {
        return Arrays.asList("age", "name", "type");
    }
    public _Person(int age, byte name[], byte type[]) {
        super();
        this.age = age;
        if ((name.length != this.name.length))
            throw new IllegalArgumentException("Wrong array size !");
        this.name = name;
        if ((type.length != this.type.length))
            throw new IllegalArgumentException("Wrong array size !");
        this.type = type;
    }
    public static class ByReference extends _Person implements Structure.ByReference {

    };
    public static class ByValue extends _Person implements Structure.ByValue {

    };
IntByReference returnCode = new IntByReference(-1);
    nativeDLL.wdatim_(new IntByReference(wdmFileNumber), new IntByReference(dataSetNumber),
            startDate, endDate, new IntByReference(), new IntByReference(), returnCode);
    if (returnCode.getValue() != 0) {
        throw new RuntimeException("WdmDll: Invalid result from call to subroutine dll.wdatim_ , returnCode = " + returnCode.getValue());
    }
*/
    int init_mtai(String config, int worker_num);
    Pointer mtai_predict(String request, int worker_idx, IntByReference bytes_write);
    void mtai_free(FreeableString str);
    int mtai_test();

    class FreeableString extends PointerType implements AutoCloseable {
        @Override
        public void close() {
            ne.mtai_free(this);
        }
        public String getString() {
            return this.getPointer().getString(0);
        }
    }
}
