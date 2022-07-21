// SPDX-FileCopyrightText: 2022 Peiwei Hu <jlu.hpw@foxmail.com>
// SPDX-License-Identifier: LGPL-3.0-only

#include <rz_core.h>

#define STRING_CHUNK 16

/**
 * Return a C/C++ string defination with block size as the length
 * \param core RzCore
 * \return a string defination or NULL if the error happens
 */
RZ_API RZ_OWN char *rz_core_print_string_c_cpp(RzCore *core) {
	ut64 value;
	size_t size = core->blocksize;
	RzStrBuf *sb = rz_strbuf_new(NULL);

	if (!sb) {
		RZ_LOG_ERROR("Fail to allocate the memory\n");
		return NULL;
	}
	rz_strbuf_appendf(sb, "#define STRING_SIZE %" PFMTSZd "\nconst char s[STRING_SIZE] = \"", size);
	for (size_t pos = 0; pos < size; pos++) {
		if (pos && !(pos % STRING_CHUNK)) {
			// newline and padding for long string
			rz_strbuf_appendf(sb, "\"\n                            \"");
		}
		value = rz_read_ble(core->block + pos, false, 8);
		rz_strbuf_appendf(sb, "\\x%02" PFMT64x, value);
	}
	rz_strbuf_append(sb, "\";");
	return rz_strbuf_drain(sb);
}

/**
 * \brief Get the hexpair of the assembly
 * \param core RzCore
 * \param assembly assembly
 * \return a string containing the hexpair of the assembly
 */
RZ_API RZ_OWN char *rz_core_hex_of_assembly(RzCore *core, const char *assembly) {
	RzStrBuf *buf = rz_strbuf_new("");
	if (!buf) {
		RZ_LOG_ERROR("Fail to allocate memory\n");
		return NULL;
	}
	rz_asm_set_pc(core->rasm, core->offset);
	RzAsmCode *acode = rz_asm_massemble(core->rasm, assembly);
	if (!acode) {
		RZ_LOG_ERROR("Fail to assemble by rz_asm_massemble()\n");
		rz_strbuf_free(buf);
		return NULL;
	}
	for (int i = 0; i < acode->len; i++) {
		ut8 b = acode->bytes[i]; // core->print->big_endian? (bytes - 1 - i): i ];
		rz_strbuf_appendf(buf, "%02x", b);
	}
	rz_asm_code_free(acode);
	return rz_strbuf_drain(buf);
}

/**
 * \brief Get the esil of the assembly
 * \param core RzCore
 * \param assembly assembly
 * \return a string containing the esil of the assembly
 */
RZ_API RZ_OWN char *rz_core_esil_of_assembly(RzCore *core, const char *assembly) {
	RzStrBuf *buf = rz_strbuf_new("");
	if (!buf) {
		RZ_LOG_ERROR("Fail to allocate memory\n");
		return NULL;
	}
	rz_asm_set_pc(core->rasm, core->offset);
	RzAsmCode *acode = rz_asm_massemble(core->rasm, assembly);
	if (!acode) {
		RZ_LOG_ERROR("Fail to assemble by rz_asm_massemble()\n");
		rz_strbuf_free(buf);
		return NULL;
	}
	int printed = 0, bufsz = acode->len;
	RzAnalysisOp aop = { 0 };
	while (printed < bufsz) {
		aop.size = 0;
		if (rz_analysis_op(core->analysis, &aop, core->offset,
			    (const ut8 *)acode->bytes + printed, bufsz - printed, RZ_ANALYSIS_OP_MASK_ESIL) <= 0 ||
			aop.size < 1) {
			RZ_LOG_ERROR("Cannot decode instruction\n");
			rz_analysis_op_fini(&aop);
			rz_strbuf_free(buf);
			rz_asm_code_free(acode);
			return NULL;
		}
		rz_strbuf_appendf(buf, "%s\n", RZ_STRBUF_SAFEGET(&aop.esil));
		printed += aop.size;
		rz_analysis_op_fini(&aop);
	}
	rz_asm_code_free(acode);
	return rz_strbuf_drain(buf);
}

/**
 * \brief Get the assembly of the hexstr
 * \param core RzCore
 * \param hex hex
 * \param len length of hex
 * \return a string containing the assembly of the hexstr
 */
RZ_API RZ_OWN char *rz_core_assembly_of_hex(RzCore *core, ut8 *hex, int len) {
	RzStrBuf *buf = rz_strbuf_new("");
	if (!buf) {
		RZ_LOG_ERROR("Fail to allocate memory\n");
		return NULL;
	}
	rz_asm_set_pc(core->rasm, core->offset);
	RzAsmCode *acode = rz_asm_mdisassemble(core->rasm, hex, len);
	if (!acode) {
		RZ_LOG_ERROR("Invalid hexstr\n");
		rz_strbuf_free(buf);
		return NULL;
	}
	rz_strbuf_append(buf, acode->assembly);
	rz_asm_code_free(acode);
	return rz_strbuf_drain(buf);
}

/**
 * \brief Get the esil of the hexstr
 * \param core RzCore
 * \param hex hex
 * \param len length of hex
 * \return a string containing the esil of the hexstr
 */
RZ_API RZ_OWN char *rz_core_esil_of_hex(RzCore *core, ut8 *hex, int len) {
	RzStrBuf *buf = rz_strbuf_new("");
	if (!buf) {
		RZ_LOG_ERROR("Fail to allocate memory\n");
		goto fail;
	}
	int printed = 0;
	RzAnalysisOp aop = { 0 };
	while (printed < len) {
		aop.size = 0;
		if (rz_analysis_op(core->analysis, &aop, core->offset,
			    (const ut8 *)hex + printed, len - printed, RZ_ANALYSIS_OP_MASK_ESIL) <= 0 ||
			aop.size < 1) {
			RZ_LOG_ERROR("Cannot decode instruction\n");
			rz_analysis_op_fini(&aop);
			goto fail;
		}
		rz_strbuf_appendf(buf, "%s\n", RZ_STRBUF_SAFEGET(&aop.esil));
		printed += aop.size;
		rz_analysis_op_fini(&aop);
	}
	return rz_strbuf_drain(buf);
fail:
	rz_strbuf_free(buf);
	return NULL;
}

/**
 * \brief Print hexdump diff between \p aa and \p ba with \p len
 */
RZ_API char *rz_core_print_hexdump_diff_str(RZ_NONNULL RzCore *core, ut64 aa, ut64 ba, ut64 len) {
	rz_return_val_if_fail(core && core->cons && len > 0, false);
	ut8 *a = malloc(len);
	if (!a) {
		return NULL;
	}
	ut8 *b = malloc(len);
	if (!b) {
		free(a);
		return NULL;
	}

	RZ_LOG_VERBOSE("print hexdump diff 0x%" PFMT64x " 0x%" PFMT64x " with len:%" PFMT64d "\n", aa, ba, len);

	rz_io_read_at(core->io, aa, a, (int)len);
	rz_io_read_at(core->io, ba, b, (int)len);
	int col = core->cons->columns > 123;
	char *pstr = rz_print_hexdiff_str(core->print, aa, a,
		ba, b, (int)len, col);
	free(a);
	free(b);
	return pstr;
}

static inline st8 format_type_to_base(const RzCorePrintFormatType format, const ut8 n) {
	static const st8 bases[][9] = {
		{ 0, 8 },
		{ 0, -1, -10, [4] = 10, [8] = -8 },
		{ 0, 16, 32, [4] = 32, [8] = 64 },
	};
	if (format >= RZ_CORE_PRINT_FORMAT_TYPE_INVALID || n >= sizeof(bases[0])) {
		return 0;
	}
	return bases[format][n];
}

static inline void fix_size_from_format(const RzCorePrintFormatType format, ut8 *size) {
	if (format != RZ_CORE_PRINT_FORMAT_TYPE_INTEGER) {
		return;
	}
	static const st8 sizes[] = {
		0, 4, 2, [4] = 4, [8] = 4
	};
	if (*size >= sizeof(sizes)) {
		return;
	}
	*size = sizes[*size];
}

static inline void len_fixup(RzCore *core, ut64 *addr, int *len) {
	if (!len) {
		return;
	}
	bool is_positive = *len > 0;
	if (RZ_ABS(*len) > core->blocksize_max) {
		RZ_LOG_ERROR("this <len> is too big (0x%" PFMT32x
			     " < 0x%" PFMT32x ").",
			*len, core->blocksize_max);
		*len = (int)core->blocksize_max;
	}
	if (is_positive) {
		return;
	}
	*len = RZ_ABS(*len);
	if (addr) {
		*addr = *addr - *len;
	}
}

/**
 * \brief Print dump at \p addr
 * \param n Word size by bytes (1,2,4,8)
 * \param len Dump bytes length
 * \param format Print format, such as RZ_CORE_PRINT_FORMAT_TYPE_HEXADECIMAL
 */
RZ_API char *rz_core_print_dump_str(RZ_NONNULL RzCore *core, RZ_NULLABLE RzCmdStateOutput *state,
	ut64 addr, ut8 n, int len, RzCorePrintFormatType format) {
	rz_return_val_if_fail(core, false);
	if (!len) {
		return NULL;
	}
	st8 base = format_type_to_base(format, n);
	if (!base) {
		return NULL;
	}
	len_fixup(core, &addr, &len);
	ut8 *buffer = malloc(len);
	if (!buffer) {
		return NULL;
	}

	char *pstr = NULL;
	rz_io_read_at(core->io, addr, buffer, len);
	rz_print_init_rowoffsets(core->print);
	core->print->use_comments = false;
	RzOutputMode mode = state ? state->mode : RZ_OUTPUT_MODE_STANDARD;

	switch (mode) {
	case RZ_OUTPUT_MODE_JSON:
		pstr = rz_print_jsondump_str(core->print, buffer, len, n * 8);
		break;
	case RZ_OUTPUT_MODE_STANDARD:
		fix_size_from_format(format, &n);
		pstr = rz_print_hexdump_str(core->print, addr,
			buffer, len, base, (int)n, 1);
		break;
	default:
		rz_warn_if_reached();
		free(buffer);
		return NULL;
	}
	free(buffer);
	return pstr;
}

/**
 * \brief Print hexdump at \p addr, but maybe print hexdiff if (diff.from or diff.to), \see "el diff"
 * \param len Dump bytes length
 */
RZ_API char *rz_core_print_hexdump_or_hexdiff_str(RZ_NONNULL RzCore *core, RZ_NULLABLE RzCmdStateOutput *state, ut64 addr, int len) {
	rz_return_val_if_fail(core, false);
	if (!len) {
		return NULL;
	}

	RzOutputMode mode = state ? state->mode : RZ_OUTPUT_MODE_STANDARD;
	char *pstr = NULL;
	switch (mode) {
	case RZ_OUTPUT_MODE_STANDARD: {
		ut64 from = rz_config_get_i(core->config, "diff.from");
		ut64 to = rz_config_get_i(core->config, "diff.to");
		if (from == to && !from) {
			len_fixup(core, &addr, &len);
			ut8 *buffer = malloc(len);
			if (!buffer) {
				return NULL;
			}
			rz_io_read_at(core->io, addr, buffer, len);
			pstr = rz_print_hexdump_str(core->print, rz_core_pava(core, addr), buffer, len, 16, 1, 1);
			free(buffer);
		} else {
			pstr = rz_core_print_hexdump_diff_str(core, addr, addr + to - from, len);
		}
		break;
	}
	case RZ_OUTPUT_MODE_JSON:
		pstr = rz_print_jsondump_str(core->print, core->block, len, 8);
		break;
	default:
		rz_warn_if_reached();
		return NULL;
	}
	return pstr;
}

static inline char *ut64_to_hex(const ut64 x, const ut8 width) {
	RzStrBuf *sb = rz_strbuf_new(NULL);
	rz_strbuf_appendf(sb, "%" PFMT64x, x);
	ut8 len = rz_strbuf_length(sb);
	if (len < width) {
		rz_strbuf_prepend(sb, rz_str_pad('0', width - len));
	}
	rz_strbuf_prepend(sb, "0x");
	return rz_strbuf_drain(sb);
}

/**
 * \brief Hexdump at \p addr
 * \param len Dump bytes length
 * \param size Word size by bytes (1,2,4,8)
 * \return Hexdump string
 */
RZ_API RZ_OWN char *rz_core_print_hexdump_byline_str(RZ_NONNULL RzCore *core, RZ_NULLABLE RzCmdStateOutput *state,
	ut64 addr, int len, ut8 size) {
	rz_return_val_if_fail(core, false);
	if (!len) {
		return NULL;
	}
	len_fixup(core, &addr, &len);
	ut8 *buffer = malloc(len);
	if (!buffer) {
		return NULL;
	}

	rz_io_read_at(core->io, addr, buffer, len);
	const int round_len = len - (len % size);
	bool hex_offset = (!(state && state->mode == RZ_OUTPUT_MODE_QUIET) && rz_config_get_i(core->config, "hex.offset"));
	RzStrBuf *sb = rz_strbuf_new(NULL);
	for (int i = 0; i < round_len; i += size) {
		const char *a, *b;
		char *fn;
		RzPrint *p = core->print;
		RzFlagItem *f;
		ut64 v = rz_read_ble(buffer + i, p->big_endian, size * 8);
		if (p->colorfor) {
			a = p->colorfor(p->user, v, true);
			if (a && *a) {
				b = Color_RESET;
			} else {
				a = b = "";
			}
		} else {
			a = b = "";
		}
		f = rz_flag_get_at(core->flags, v, true);
		fn = NULL;
		if (f) {
			st64 delta = (st64)(v - f->offset);
			if (delta >= 0 && delta < 8192) {
				if (v == f->offset) {
					fn = strdup(f->name);
				} else {
					fn = rz_str_newf("%s+%" PFMT64d, f->name, v - f->offset);
				}
			}
		}
		char *vstr = ut64_to_hex(v, size * 2);
		if (vstr) {
			if (hex_offset) {
				rz_strbuf_append(sb, rz_print_section_str(core->print, addr + i));
				rz_strbuf_appendf(sb, "0x%08" PFMT64x " %s%s%s%s%s\n",
					(ut64)addr + i, a, vstr, b, fn ? " " : "", fn ? fn : "");
			} else {
				rz_strbuf_appendf(sb, "%s%s%s\n", a, vstr, b);
			}
		}
		free(vstr);
		free(fn);
	}
	free(buffer);
	return rz_strbuf_drain(sb);
}
