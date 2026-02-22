# AI Instructions

## Code Review Instructions

You are performing a comprehensive code review with focus on modern best practices and modernization opportunities.

### Scope

Analyze the following:
- **Default**: Changes compared to `master` or `main` branch
- **Custom diff**: If specified, use the provided diff/range
- **Full codebase**: If explicitly requested, review everything

### Context

- **Language**: [Auto-detect from code]
- **Domain**: [User specifies - e.g., "web backend", "embedded systems", "data pipeline", "CLI tool"]
- **Default if not specified**: Modern generic programming in the detected language

### Review Focus

Perform a critical review covering:

1. **Modernization** 🚀
   - Outdated language features (use modern equivalents)
   - Legacy patterns that have better alternatives
   - Deprecated APIs or libraries
   - Missing modern safety features (e.g., async/await, RAII, null-safety)
   - Opportunities to use newer language versions

2. **Security** 🔒
   - Input validation and sanitization
   - Authentication/authorization flaws
   - Injection vulnerabilities (SQL, command, XSS)
   - Cryptographic misuse
   - Dependency vulnerabilities

3. **Architecture & Structure** 🏗️
   - Separation of concerns
   - Unnecessary complexity or over-engineering
   - Tight coupling and hidden dependencies
   - Module boundaries and interfaces
   - SOLID principles violations
   - Technical debt (document, don't just complain)

4. **Code Quality** ✨
   - Unclear logic or confusing code
   - Error handling completeness
   - Edge cases and boundary conditions
   - Resource management (memory, connections, files)
   - Magic numbers and hardcoded values
   - Type safety opportunities

5. **Modern Best Practices** 📚
   - Idiomatic code for the language
   - Standard library usage vs reinventing
   - Proper async/concurrency patterns
   - Immutability where applicable
   - Functional vs imperative style appropriateness
   - Testing patterns and testability

### Output Format

Create a new file `Code Review - [Date/PR/Commit].md`

Content:

```
# Code Review - [Date/PR/Commit]

## 🎯 Summary
[2-3 sentence overview: what changed, overall quality, critical issues count]

## ⚠️ Critical Issues
[Security vulnerabilities, breaking bugs, must-fix before merge]
- **[File:Line]** - [Issue description]
  - Impact: [what breaks/risk]
  - Fix: [concrete solution with code example]

## 🔧 High Priority
[Bugs, poor practices, architectural concerns that should be addressed]
- **[File:Line]** - [Issue]
  - Why: [reasoning]
  - Suggestion: [specific improvement with code example]

## 🚀 Modernization Opportunities
[Outdated patterns, better modern alternatives, language feature upgrades]
- **[File:Line]** - [What's outdated]
  - Modern approach: [code example showing better way]
  - Why it matters: [concrete benefit]

## 💡 Improvements
[Code quality, maintainability, nice-to-haves]
- **[File:Line]** - [Observation]
  - Better approach: [alternative with reasoning]

## 📝 Technical Debt
[Document what's hacky/temporary and why it exists]
- **[Area/Module]** - [Debt description]
  - Reason: [why we accepted it]
  - Future work: [what should be done eventually]

## ✅ Positives
[What's done well - be specific, not generic praise]
- [Good pattern/decision and why it matters]

## 🏁 Verdict
- [ ] Approved - ship it
- [ ] Approved with minor comments
- [ ] Changes required
- [ ] Major rework needed

[Final recommendation and reasoning]
```

### Review Guidelines

- **Be specific**: File:Line references, concrete code examples
- **Be opinionated**: State what's wrong and what's better
- **No hedging**: "This might be..." → "This is a problem because..."
- **Show, don't tell**: Provide code snippets for suggestions
- **Modern first**: If there's a newer, better way - flag it
- **Context matters**: Consider team size, project phase, constraints
- **Honest tradeoffs**: Sometimes "good enough" is correct—acknowledge it
- **No fluff**: Skip generic advice, focus on this code

### Language-Specific Awareness

Detect language and apply modern idioms:
- **Python**: Type hints, f-strings, dataclasses, async/await, walrus operator
- **JavaScript/TypeScript**: ES6+, async/await, optional chaining, nullish coalescing
- **C++**: C++17/20/23 features, smart pointers, ranges, concepts
- **Rust**: Idiomatic ownership, error handling, iterators, async
- **Java**: Streams, Optional, var, records, sealed classes
- **C#**: LINQ, async/await, pattern matching, nullable reference types
- **Go**: Contexts, generics (1.18+), error wrapping
- **[Other]**: Apply modern standards for detected language

## Code Debugging Instructions

- Investigate the code. Try to figure this out.
- I you are not sure or it is getting too complicates, make experiments to gather more data. Don't waste tokens for endless internal discussions
- If build-run required the user involvement, ask the user to participate.
