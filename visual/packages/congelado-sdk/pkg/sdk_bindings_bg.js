export class IntoUnderlyingByteSource {
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        IntoUnderlyingByteSourceFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        wasm.__wbg_intounderlyingbytesource_free(ptr, 0);
    }
    /**
     * @returns {number}
     */
    get autoAllocateChunkSize() {
        const ret = wasm.intounderlyingbytesource_autoAllocateChunkSize(this.__wbg_ptr);
        return ret >>> 0;
    }
    cancel() {
        const ptr = this.__destroy_into_raw();
        wasm.intounderlyingbytesource_cancel(ptr);
    }
    /**
     * @param {ReadableByteStreamController} controller
     * @returns {Promise<any>}
     */
    pull(controller) {
        const ret = wasm.intounderlyingbytesource_pull(this.__wbg_ptr, controller);
        return ret;
    }
    /**
     * @param {ReadableByteStreamController} controller
     */
    start(controller) {
        wasm.intounderlyingbytesource_start(this.__wbg_ptr, controller);
    }
    /**
     * @returns {ReadableStreamType}
     */
    get type() {
        const ret = wasm.intounderlyingbytesource_type(this.__wbg_ptr);
        return __wbindgen_enum_ReadableStreamType[ret];
    }
}
if (Symbol.dispose) IntoUnderlyingByteSource.prototype[Symbol.dispose] = IntoUnderlyingByteSource.prototype.free;

export class IntoUnderlyingSink {
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        IntoUnderlyingSinkFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        wasm.__wbg_intounderlyingsink_free(ptr, 0);
    }
    /**
     * @param {any} reason
     * @returns {Promise<any>}
     */
    abort(reason) {
        const ptr = this.__destroy_into_raw();
        const ret = wasm.intounderlyingsink_abort(ptr, reason);
        return ret;
    }
    /**
     * @returns {Promise<any>}
     */
    close() {
        const ptr = this.__destroy_into_raw();
        const ret = wasm.intounderlyingsink_close(ptr);
        return ret;
    }
    /**
     * @param {any} chunk
     * @returns {Promise<any>}
     */
    write(chunk) {
        const ret = wasm.intounderlyingsink_write(this.__wbg_ptr, chunk);
        return ret;
    }
}
if (Symbol.dispose) IntoUnderlyingSink.prototype[Symbol.dispose] = IntoUnderlyingSink.prototype.free;

export class IntoUnderlyingSource {
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        IntoUnderlyingSourceFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        wasm.__wbg_intounderlyingsource_free(ptr, 0);
    }
    cancel() {
        const ptr = this.__destroy_into_raw();
        wasm.intounderlyingsource_cancel(ptr);
    }
    /**
     * @param {ReadableStreamDefaultController} controller
     * @returns {Promise<any>}
     */
    pull(controller) {
        const ret = wasm.intounderlyingsource_pull(this.__wbg_ptr, controller);
        return ret;
    }
}
if (Symbol.dispose) IntoUnderlyingSource.prototype[Symbol.dispose] = IntoUnderlyingSource.prototype.free;

/**
 * The SDK handle imported by `visual` packages.
 */
export class Sdk {
    __destroy_into_raw() {
        const ptr = this.__wbg_ptr;
        this.__wbg_ptr = 0;
        SdkFinalization.unregister(this);
        return ptr;
    }
    free() {
        const ptr = this.__destroy_into_raw();
        wasm.__wbg_sdk_free(ptr, 0);
    }
    /**
     * Returns the configured server base URL.
     * @returns {string}
     */
    baseUrl() {
        let deferred1_0;
        let deferred1_1;
        try {
            const ret = wasm.sdk_baseUrl(this.__wbg_ptr);
            deferred1_0 = ret[0];
            deferred1_1 = ret[1];
            return getStringFromWasm0(ret[0], ret[1]);
        } finally {
            wasm.__wbindgen_free(deferred1_0, deferred1_1, 1);
        }
    }
    /**
     * Fetches posts through the OpenAPI-generated request method.
     * @returns {Promise<any>}
     */
    listPosts() {
        const ret = wasm.sdk_listPosts(this.__wbg_ptr);
        return ret;
    }
    /**
     * Creates a client for `base_url`.
     * @param {string} base_url
     */
    constructor(base_url) {
        const ptr0 = passStringToWasm0(base_url, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
        const len0 = WASM_VECTOR_LEN;
        const ret = wasm.sdk_new(ptr0, len0);
        if (ret[2]) {
            throw takeFromExternrefTable0(ret[1]);
        }
        this.__wbg_ptr = ret[0];
        SdkFinalization.register(this, this.__wbg_ptr, this);
        return this;
    }
}
if (Symbol.dispose) Sdk.prototype[Symbol.dispose] = Sdk.prototype.free;
export function __wbg___wbindgen_debug_string_a57024b9c6e4a48b(arg0, arg1) {
    const ret = debugString(arg1);
    const ptr1 = passStringToWasm0(ret, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
    const len1 = WASM_VECTOR_LEN;
    getDataViewMemory0().setInt32(arg0 + 4 * 1, len1, true);
    getDataViewMemory0().setInt32(arg0 + 4 * 0, ptr1, true);
}
export function __wbg___wbindgen_is_function_5e4570eb24ffa122(arg0) {
    const ret = typeof(arg0) === 'function';
    return ret;
}
export function __wbg___wbindgen_is_undefined_6cff064c44e0d823(arg0) {
    const ret = arg0 === undefined;
    return ret;
}
export function __wbg___wbindgen_string_get_d154f1e671052120(arg0, arg1) {
    const obj = arg1;
    const ret = typeof(obj) === 'string' ? obj : undefined;
    var ptr1 = isLikeNone(ret) ? 0 : passStringToWasm0(ret, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
    var len1 = WASM_VECTOR_LEN;
    getDataViewMemory0().setInt32(arg0 + 4 * 1, len1, true);
    getDataViewMemory0().setInt32(arg0 + 4 * 0, ptr1, true);
}
export function __wbg___wbindgen_throw_bb96b2010945f0bc(arg0, arg1) {
    throw new Error(getStringFromWasm0(arg0, arg1));
}
export function __wbg__wbg_cb_unref_be22cc64ae6946a0(arg0) {
    arg0._wbg_cb_unref();
}
export function __wbg_abort_1254fe4ac5695dc0(arg0, arg1) {
    arg0.abort(arg1);
}
export function __wbg_abort_d8615b5857e112b3(arg0) {
    arg0.abort();
}
export function __wbg_append_acad6a3f39a3e778() { return handleError(function (arg0, arg1, arg2, arg3, arg4) {
    arg0.append(getStringFromWasm0(arg1, arg2), getStringFromWasm0(arg3, arg4));
}, arguments); }
export function __wbg_buffer_78291c0e094ccf99(arg0) {
    const ret = arg0.buffer;
    return ret;
}
export function __wbg_byobRequest_f8b1c89429b77545(arg0) {
    const ret = arg0.byobRequest;
    return isLikeNone(ret) ? 0 : addToExternrefTable0(ret);
}
export function __wbg_byteLength_336bc7d303511ba0(arg0) {
    const ret = arg0.byteLength;
    return ret;
}
export function __wbg_byteOffset_2b1d5b10453ce198(arg0) {
    const ret = arg0.byteOffset;
    return ret;
}
export function __wbg_call_35dba3c747ad7521() { return handleError(function (arg0, arg1, arg2) {
    const ret = arg0.call(arg1, arg2);
    return ret;
}, arguments); }
export function __wbg_clearTimeout_1daad5c6ee4fcd3b(arg0) {
    const ret = clearTimeout(arg0);
    return ret;
}
export function __wbg_close_72f69f5f2de2bc73() { return handleError(function (arg0) {
    arg0.close();
}, arguments); }
export function __wbg_close_97cdb44c3a7878f6() { return handleError(function (arg0) {
    arg0.close();
}, arguments); }
export function __wbg_done_669171204c3dcae2(arg0) {
    const ret = arg0.done;
    return ret;
}
export function __wbg_enqueue_7d68a21eda78e72f() { return handleError(function (arg0, arg1) {
    arg0.enqueue(arg1);
}, arguments); }
export function __wbg_entries_2c710161cbd65b89(arg0) {
    const ret = arg0.entries();
    return ret;
}
export function __wbg_fetch_7a37768ab67c5bd8(arg0) {
    const ret = fetch(arg0);
    return ret;
}
export function __wbg_fetch_d752d93f5b259503(arg0, arg1) {
    const ret = arg0.fetch(arg1);
    return ret;
}
export function __wbg_get_c0c8f8d7da0c03dd(arg0, arg1) {
    const ret = arg0[arg1 >>> 0];
    return ret;
}
export function __wbg_has_b3a6e6d0d28295fa() { return handleError(function (arg0, arg1) {
    const ret = Reflect.has(arg0, arg1);
    return ret;
}, arguments); }
export function __wbg_headers_92567b07014384b9(arg0) {
    const ret = arg0.headers;
    return ret;
}
export function __wbg_instanceof_Response_8f49efbd4bfd76d6(arg0) {
    let result;
    try {
        result = arg0 instanceof Response;
    } catch (_) {
        result = false;
    }
    const ret = result;
    return ret;
}
export function __wbg_isArray_6339f732981044bf(arg0) {
    const ret = Array.isArray(arg0);
    return ret;
}
export function __wbg_length_36bd29c6848c2144(arg0) {
    const ret = arg0.length;
    return ret;
}
export function __wbg_new_358857d90afd5a2d(arg0, arg1) {
    const ret = new Error(getStringFromWasm0(arg0, arg1));
    return ret;
}
export function __wbg_new_95039e162b0c4466() { return handleError(function () {
    const ret = new Headers();
    return ret;
}, arguments); }
export function __wbg_new_ebe3e0f6837f0879() {
    const ret = new Object();
    return ret;
}
export function __wbg_new_f5712de39c931ddf() { return handleError(function () {
    const ret = new AbortController();
    return ret;
}, arguments); }
export function __wbg_new_from_slice_3eea173078478cfe(arg0, arg1) {
    const ret = new Uint8Array(getArrayU8FromWasm0(arg0, arg1));
    return ret;
}
export function __wbg_new_typed_cceaf62d8d95e9f2(arg0, arg1) {
    try {
        var state0 = {a: arg0, b: arg1};
        var cb0 = (arg0, arg1) => {
            const a = state0.a;
            state0.a = 0;
            try {
                return wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___js_sys_7de23dd41b27dfd2___Function_fn_wasm_bindgen_1f704a91cc1dcf17___JsValue_____wasm_bindgen_1f704a91cc1dcf17___sys__Undefined___js_sys_7de23dd41b27dfd2___Function_fn_wasm_bindgen_1f704a91cc1dcf17___JsValue_____wasm_bindgen_1f704a91cc1dcf17___sys__Undefined_______true_(a, state0.b, arg0, arg1);
            } finally {
                state0.a = a;
            }
        };
        const ret = new Promise(cb0);
        return ret;
    } finally {
        state0.a = 0;
    }
}
export function __wbg_new_with_byte_offset_and_length_ff6e927f8d72f0c3(arg0, arg1, arg2) {
    const ret = new Uint8Array(arg0, arg1 >>> 0, arg2 >>> 0);
    return ret;
}
export function __wbg_new_with_str_and_init_5a37d576dec75a86() { return handleError(function (arg0, arg1, arg2) {
    const ret = new Request(getStringFromWasm0(arg0, arg1), arg2);
    return ret;
}, arguments); }
export function __wbg_next_42cf16ee0dafc9e2() { return handleError(function (arg0) {
    const ret = arg0.next();
    return ret;
}, arguments); }
export function __wbg_queueMicrotask_ac694eae12e92dfb(arg0) {
    queueMicrotask(arg0);
}
export function __wbg_queueMicrotask_be5fe34a8f4cad4d(arg0) {
    const ret = arg0.queueMicrotask;
    return ret;
}
export function __wbg_resolve_020f95d838c6ef25(arg0) {
    const ret = Promise.resolve(arg0);
    return ret;
}
export function __wbg_respond_f88cbcebace42068() { return handleError(function (arg0, arg1) {
    arg0.respond(arg1 >>> 0);
}, arguments); }
export function __wbg_run_ef366b557a6598c4(arg0, arg1, arg2) {
    try {
        var state0 = {a: arg1, b: arg2};
        var cb0 = () => {
            const a = state0.a;
            state0.a = 0;
            try {
                return wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___bool__true_(a, state0.b, );
            } finally {
                state0.a = a;
            }
        };
        const ret = arg0.run(cb0);
        return ret;
    } finally {
        state0.a = 0;
    }
}
export function __wbg_setTimeout_b4a9096784635514(arg0, arg1) {
    const ret = setTimeout(arg0, arg1);
    return ret;
}
export function __wbg_set_b9b5b5cb7b495037(arg0, arg1, arg2) {
    arg0.set(getArrayU8FromWasm0(arg1, arg2));
}
export function __wbg_set_body_f301b68bff45f419(arg0, arg1) {
    arg0.body = arg1;
}
export function __wbg_set_cache_ab8f11813716fe29(arg0, arg1) {
    arg0.cache = __wbindgen_enum_RequestCache[arg1];
}
export function __wbg_set_credentials_d7f3b810cbf191e1(arg0, arg1) {
    arg0.credentials = __wbindgen_enum_RequestCredentials[arg1];
}
export function __wbg_set_headers_805555608daf7f2a(arg0, arg1) {
    arg0.headers = arg1;
}
export function __wbg_set_method_cf2b992b9a610bc3(arg0, arg1, arg2) {
    arg0.method = getStringFromWasm0(arg1, arg2);
}
export function __wbg_set_mode_d6479dfd6696c8d3(arg0, arg1) {
    arg0.mode = __wbindgen_enum_RequestMode[arg1];
}
export function __wbg_set_referrer_de24582cadb58b95(arg0, arg1, arg2) {
    arg0.referrer = getStringFromWasm0(arg1, arg2);
}
export function __wbg_set_referrer_policy_3f990097931e92b7(arg0, arg1) {
    arg0.referrerPolicy = __wbindgen_enum_ReferrerPolicy[arg1];
}
export function __wbg_set_signal_115b9e9423652e66(arg0, arg1) {
    arg0.signal = arg1;
}
export function __wbg_signal_58449b7eb331d1be(arg0) {
    const ret = arg0.signal;
    return ret;
}
export function __wbg_static_accessor_CREATE_TASK_307e3054ac4aa976() {
    const ret = typeof console === 'undefined' ? null : console?.createTask;
    return isLikeNone(ret) ? 0 : addToExternrefTable0(ret);
}
export function __wbg_static_accessor_GLOBAL_THIS_466428f93b4eaa76() {
    const ret = typeof globalThis === 'undefined' ? null : globalThis;
    return isLikeNone(ret) ? 0 : addToExternrefTable0(ret);
}
export function __wbg_static_accessor_GLOBAL_c7aea38d4de089bc() {
    const ret = typeof global === 'undefined' ? null : global;
    return isLikeNone(ret) ? 0 : addToExternrefTable0(ret);
}
export function __wbg_static_accessor_SELF_42d4fae05e59267a() {
    const ret = typeof self === 'undefined' ? null : self;
    return isLikeNone(ret) ? 0 : addToExternrefTable0(ret);
}
export function __wbg_static_accessor_WINDOW_e0db14a0eba6a812() {
    const ret = typeof window === 'undefined' ? null : window;
    return isLikeNone(ret) ? 0 : addToExternrefTable0(ret);
}
export function __wbg_status_b0de02a07fd7d927(arg0) {
    const ret = arg0.status;
    return ret;
}
export function __wbg_then_7026b513a94278a8(arg0, arg1) {
    const ret = arg0.then(arg1);
    return ret;
}
export function __wbg_then_72819b8d4e081fb5(arg0, arg1, arg2) {
    const ret = arg0.then(arg1, arg2);
    return ret;
}
export function __wbg_url_82c95d5d2e2ba977(arg0, arg1) {
    const ret = arg1.url;
    const ptr1 = passStringToWasm0(ret, wasm.__wbindgen_malloc, wasm.__wbindgen_realloc);
    const len1 = WASM_VECTOR_LEN;
    getDataViewMemory0().setInt32(arg0 + 4 * 1, len1, true);
    getDataViewMemory0().setInt32(arg0 + 4 * 0, ptr1, true);
}
export function __wbg_value_1e2369fab29b420e(arg0) {
    const ret = arg0.value;
    return ret;
}
export function __wbg_view_7685fe4b2845c5b6(arg0) {
    const ret = arg0.view;
    return isLikeNone(ret) ? 0 : addToExternrefTable0(ret);
}
export function __wbindgen_cast_0000000000000001(arg0, arg1) {
    // Cast intrinsic for `Closure(Closure { owned: true, function: Function { arguments: [Externref], shim_idx: 274, ret: Result(Unit), inner_ret: Some(Result(Unit)) }, mutable: true }) -> Externref`.
    const ret = makeMutClosure(arg0, arg1, wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___wasm_bindgen_1f704a91cc1dcf17___JsValue__core_37ee0ce0c4cd57d5___result__Result_____wasm_bindgen_1f704a91cc1dcf17___JsError___true_);
    return ret;
}
export function __wbindgen_cast_0000000000000002(arg0, arg1) {
    // Cast intrinsic for `Closure(Closure { owned: true, function: Function { arguments: [], shim_idx: 77, ret: Unit, inner_ret: Some(Unit) }, mutable: true }) -> Externref`.
    const ret = makeMutClosure(arg0, arg1, wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke_______true_);
    return ret;
}
export function __wbindgen_cast_0000000000000003(arg0, arg1) {
    // Cast intrinsic for `Ref(String) -> Externref`.
    const ret = getStringFromWasm0(arg0, arg1);
    return ret;
}
export function __wbindgen_init_externref_table() {
    const table = wasm.__wbindgen_externrefs;
    const offset = table.grow(4);
    table.set(0, undefined);
    table.set(offset + 0, undefined);
    table.set(offset + 1, null);
    table.set(offset + 2, true);
    table.set(offset + 3, false);
}
function wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke_______true_(arg0, arg1) {
    wasm.wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke_______true_(arg0, arg1);
}

function wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___bool__true_(arg0, arg1) {
    const ret = wasm.wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___bool__true_(arg0, arg1);
    return ret !== 0;
}

function wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___wasm_bindgen_1f704a91cc1dcf17___JsValue__core_37ee0ce0c4cd57d5___result__Result_____wasm_bindgen_1f704a91cc1dcf17___JsError___true_(arg0, arg1, arg2) {
    const ret = wasm.wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___wasm_bindgen_1f704a91cc1dcf17___JsValue__core_37ee0ce0c4cd57d5___result__Result_____wasm_bindgen_1f704a91cc1dcf17___JsError___true_(arg0, arg1, arg2);
    if (ret[1]) {
        throw takeFromExternrefTable0(ret[0]);
    }
}

function wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___js_sys_7de23dd41b27dfd2___Function_fn_wasm_bindgen_1f704a91cc1dcf17___JsValue_____wasm_bindgen_1f704a91cc1dcf17___sys__Undefined___js_sys_7de23dd41b27dfd2___Function_fn_wasm_bindgen_1f704a91cc1dcf17___JsValue_____wasm_bindgen_1f704a91cc1dcf17___sys__Undefined_______true_(arg0, arg1, arg2, arg3) {
    wasm.wasm_bindgen_1f704a91cc1dcf17___convert__closures_____invoke___js_sys_7de23dd41b27dfd2___Function_fn_wasm_bindgen_1f704a91cc1dcf17___JsValue_____wasm_bindgen_1f704a91cc1dcf17___sys__Undefined___js_sys_7de23dd41b27dfd2___Function_fn_wasm_bindgen_1f704a91cc1dcf17___JsValue_____wasm_bindgen_1f704a91cc1dcf17___sys__Undefined_______true_(arg0, arg1, arg2, arg3);
}


const __wbindgen_enum_ReadableStreamType = ["bytes"];


const __wbindgen_enum_ReferrerPolicy = ["", "no-referrer", "no-referrer-when-downgrade", "origin", "origin-when-cross-origin", "unsafe-url", "same-origin", "strict-origin", "strict-origin-when-cross-origin"];


const __wbindgen_enum_RequestCache = ["default", "no-store", "reload", "no-cache", "force-cache", "only-if-cached"];


const __wbindgen_enum_RequestCredentials = ["omit", "same-origin", "include"];


const __wbindgen_enum_RequestMode = ["same-origin", "no-cors", "cors", "navigate"];
const IntoUnderlyingByteSourceFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(ptr => wasm.__wbg_intounderlyingbytesource_free(ptr, 1));
const IntoUnderlyingSinkFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(ptr => wasm.__wbg_intounderlyingsink_free(ptr, 1));
const IntoUnderlyingSourceFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(ptr => wasm.__wbg_intounderlyingsource_free(ptr, 1));
const SdkFinalization = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(ptr => wasm.__wbg_sdk_free(ptr, 1));

function addToExternrefTable0(obj) {
    const idx = wasm.__externref_table_alloc();
    wasm.__wbindgen_externrefs.set(idx, obj);
    return idx;
}

const CLOSURE_DTORS = (typeof FinalizationRegistry === 'undefined')
    ? { register: () => {}, unregister: () => {} }
    : new FinalizationRegistry(state => wasm.__wbindgen_destroy_closure(state.a, state.b));

function debugString(val) {
    // primitive types
    const type = typeof val;
    if (type == 'number' || type == 'boolean' || val == null) {
        return  `${val}`;
    }
    if (type == 'string') {
        return `"${val}"`;
    }
    if (type == 'symbol') {
        const description = val.description;
        if (description == null) {
            return 'Symbol';
        } else {
            return `Symbol(${description})`;
        }
    }
    if (type == 'function') {
        const name = val.name;
        if (typeof name == 'string' && name.length > 0) {
            return `Function(${name})`;
        } else {
            return 'Function';
        }
    }
    // objects
    if (Array.isArray(val)) {
        const length = val.length;
        let debug = '[';
        if (length > 0) {
            debug += debugString(val[0]);
        }
        for(let i = 1; i < length; i++) {
            debug += ', ' + debugString(val[i]);
        }
        debug += ']';
        return debug;
    }
    // Test for built-in
    const builtInMatches = /\[object ([^\]]+)\]/.exec(toString.call(val));
    let className;
    if (builtInMatches && builtInMatches.length > 1) {
        className = builtInMatches[1];
    } else {
        // Failed to match the standard '[object ClassName]'
        return toString.call(val);
    }
    if (className == 'Object') {
        // we're a user defined class or Object
        // JSON.stringify avoids problems with cycles, and is generally much
        // easier than looping through ownProperties of `val`.
        try {
            return 'Object(' + JSON.stringify(val) + ')';
        } catch (_) {
            return 'Object';
        }
    }
    // errors
    if (val instanceof Error) {
        return `${val.name}: ${val.message}\n${val.stack}`;
    }
    // TODO we could test for more things here, like `Set`s and `Map`s.
    return className;
}

function getArrayU8FromWasm0(ptr, len) {
    ptr = ptr >>> 0;
    return getUint8ArrayMemory0().subarray(ptr / 1, ptr / 1 + len);
}

let cachedDataViewMemory0 = null;
function getDataViewMemory0() {
    if (cachedDataViewMemory0 === null || cachedDataViewMemory0.buffer.detached === true || (cachedDataViewMemory0.buffer.detached === undefined && cachedDataViewMemory0.buffer !== wasm.memory.buffer)) {
        cachedDataViewMemory0 = new DataView(wasm.memory.buffer);
    }
    return cachedDataViewMemory0;
}

function getStringFromWasm0(ptr, len) {
    return decodeText(ptr >>> 0, len);
}

let cachedUint8ArrayMemory0 = null;
function getUint8ArrayMemory0() {
    if (cachedUint8ArrayMemory0 === null || cachedUint8ArrayMemory0.byteLength === 0) {
        cachedUint8ArrayMemory0 = new Uint8Array(wasm.memory.buffer);
    }
    return cachedUint8ArrayMemory0;
}

function handleError(f, args) {
    try {
        return f.apply(this, args);
    } catch (e) {
        const idx = addToExternrefTable0(e);
        wasm.__wbindgen_exn_store(idx);
    }
}

function isLikeNone(x) {
    return x === undefined || x === null;
}

function makeMutClosure(arg0, arg1, f) {
    const state = { a: arg0, b: arg1, cnt: 1 };
    const real = (...args) => {

        // First up with a closure we increment the internal reference
        // count. This ensures that the Rust closure environment won't
        // be deallocated while we're invoking it.
        state.cnt++;
        const a = state.a;
        state.a = 0;
        try {
            return f(a, state.b, ...args);
        } finally {
            state.a = a;
            real._wbg_cb_unref();
        }
    };
    real._wbg_cb_unref = () => {
        if (--state.cnt === 0) {
            wasm.__wbindgen_destroy_closure(state.a, state.b);
            state.a = 0;
            CLOSURE_DTORS.unregister(state);
        }
    };
    CLOSURE_DTORS.register(real, state, state);
    return real;
}

function passStringToWasm0(arg, malloc, realloc) {
    if (realloc === undefined) {
        const buf = cachedTextEncoder.encode(arg);
        const ptr = malloc(buf.length, 1) >>> 0;
        getUint8ArrayMemory0().subarray(ptr, ptr + buf.length).set(buf);
        WASM_VECTOR_LEN = buf.length;
        return ptr;
    }

    let len = arg.length;
    let ptr = malloc(len, 1) >>> 0;

    const mem = getUint8ArrayMemory0();

    let offset = 0;

    for (; offset < len; offset++) {
        const code = arg.charCodeAt(offset);
        if (code > 0x7F) break;
        mem[ptr + offset] = code;
    }
    if (offset !== len) {
        if (offset !== 0) {
            arg = arg.slice(offset);
        }
        ptr = realloc(ptr, len, len = offset + arg.length * 3, 1) >>> 0;
        const view = getUint8ArrayMemory0().subarray(ptr + offset, ptr + len);
        const ret = cachedTextEncoder.encodeInto(arg, view);

        offset += ret.written;
        ptr = realloc(ptr, len, offset, 1) >>> 0;
    }

    WASM_VECTOR_LEN = offset;
    return ptr;
}

function takeFromExternrefTable0(idx) {
    const value = wasm.__wbindgen_externrefs.get(idx);
    wasm.__externref_table_dealloc(idx);
    return value;
}

let cachedTextDecoder = new TextDecoder('utf-8', { ignoreBOM: true, fatal: true });
cachedTextDecoder.decode();
const MAX_SAFARI_DECODE_BYTES = 2146435072;
let numBytesDecoded = 0;
function decodeText(ptr, len) {
    numBytesDecoded += len;
    if (numBytesDecoded >= MAX_SAFARI_DECODE_BYTES) {
        cachedTextDecoder = new TextDecoder('utf-8', { ignoreBOM: true, fatal: true });
        cachedTextDecoder.decode();
        numBytesDecoded = len;
    }
    return cachedTextDecoder.decode(getUint8ArrayMemory0().subarray(ptr, ptr + len));
}

const cachedTextEncoder = new TextEncoder();

if (!('encodeInto' in cachedTextEncoder)) {
    cachedTextEncoder.encodeInto = function (arg, view) {
        const buf = cachedTextEncoder.encode(arg);
        view.set(buf);
        return {
            read: arg.length,
            written: buf.length
        };
    };
}

let WASM_VECTOR_LEN = 0;


let wasm;
export function __wbg_set_wasm(val) {
    wasm = val;
}
