# Code Review - 2026-02-25

Branch: `fix/winarm64_vs_processing` → `main`
Commit: `829dfcc Fix va handling on Windows on ARM causing crashes in some configurations`
Scope: `src/ulog.c` — 20 lines changed

---

## 🎯 Summary

Three targeted fixes for `va_list` misuse that caused crashes on Windows ARM64 (MSVC). The changes cover `ulog_log`, `ulog_event_get_message`, and `ulog_event_to_cstr`. Code is correct and focused; no unnecessary scope creep.

---

## ⚠️ Critical Issues

None.

---

## 🔧 High Priority

None.

---

## 🚀 Modernization Opportunities

Not applicable — this is C with platform-portability constraints.

---

## 💡 Improvements

**[src/ulog.c:335, 1773]** — Comment slightly overstates cost predictability

The comment `"~4-5 word moves"` is inaccurate when optional features are compiled in. With `ULOG_HAS_TOPICS`, `ULOG_HAS_TIME`, and `ULOG_HAS_SOURCE_LOCATION` all enabled, `sizeof(ulog_event)` grows to ~8–10 words (2 `int`s, 3–4 pointers, plus `va_list`). The claim is true for a minimal build but misleading for a full one.

- Better: `"memcpy of a small stack struct; vsnprintf below dominates"` — or just drop the word count.

**[src/ulog.c:336, 1774]** — Zero-init before `memcpy` is redundant

```c
ulog_event ev_copy = {0};
memcpy(&ev_copy, ev, sizeof(ulog_event));
```

`memcpy` overwrites every byte anyway, so the `= {0}` buys nothing. It's not wrong, but it creates a false impression that the zeroing matters for correctness. The only byte that is not covered by `memcpy` but needs initialization is `va_list` — and that is handled by `va_copy` immediately after.

If the intent is defensive clarity, a comment explaining why would be better than silent double-initialization.

---

## 📝 Technical Debt

**[src/ulog.c — va_list in struct]** — Root fragility: `va_list` stored inside a struct

Storing a `va_list` inside `ulog_event` is the underlying source of all three fixes. The C standard permits it, but it creates a recurring friction point:

- Struct copy of a `va_list`-containing struct is always risky (hence the `memcpy` change).
- `va_start` directly into a struct member is non-portable (hence the intermediate `args` change).
- Every new call site must remember to `va_copy`/`va_end` correctly.

Future work: Consider storing the formatted string directly in `ulog_event` (fixed-size buffer or pointer to caller-owned buffer) instead of keeping the raw `va_list` alive. This would eliminate all `va_list` lifecycle concerns from the event struct at the cost of one early `vsnprintf` call.

---

## ✅ Positives

- **`ulog_log` fix is textbook-correct.** Using an intermediate `va_list`, copying with `va_copy`, then immediately calling `va_end` on the source is exactly the right pattern. It also properly calls `va_end(args)` which the original pattern sidestepped.

- **Consistent fix across all three sites.** Both `ulog_event_get_message` and `ulog_event_to_cstr` received the identical treatment, preventing latent bugs in the other paths.

- **No behavior change for correct platforms.** The fix is purely mechanical; there is no logic change, making regression risk minimal.

- **Branch name and commit message are accurate and descriptive.** Easy to bisect later.

---

## 🏁 Verdict

- [x] Approved — ship it

The fixes are correct, minimal, and well-scoped. The improvements noted above are cosmetic (comment wording, redundant zero-init) and do not block merge. The technical debt item is a pre-existing design choice, not introduced by this PR.
