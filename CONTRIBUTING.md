# Contributing Guide

Thank you for your interest in improving the Brainiacs Autonomous Vehicle project!

## Ways to Contribute
- Documentation: clarify setup, add diagrams, calibration notes.
- Code: performance, reliability, new sensors, safety features.
- Testing: create reproducible test tracks & data logs.
- Hardware: lighter chassis, improved power delivery.

## Ground Rules
- Keep commits scoped and descriptive.
- Open an issue before large refactors.
- Follow existing naming conventions.
- Document new dependencies in README or dedicated doc.

## Development Workflow
1. Fork and create a feature branch: `feat/<short-topic>`
2. Run & test locally (simulate sensor inputs where possible).
3. Open a Pull Request referencing any related issue.
4. Respond to review feedback and keep PR focused.

## Code Style
- Arduino: keep loop deterministic; avoid long blocking delays.
- Python: PEP 8; favor small, testable functions.

## Reporting Issues
Include: steps to reproduce, expected vs actual behavior, logs / serial output snippets, photos if hardware related.

## License
Specify project license in `LICENSE` (to be added) so contributors understand usage rights.
