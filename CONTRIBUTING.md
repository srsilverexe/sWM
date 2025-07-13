# Contributing Guidelines

We welcome contributions! Please follow these guidelines:

## Reporting Issues
- Check existing issues before opening new ones
- Include detailed reproduction steps
- Provide Xorg log snippets if possible

## Code Contributions
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to branch (`git push origin feature/your-feature`)
5. Open a Pull Request

## Coding Standards
- Follow existing code style (4-space indentation)
- Write descriptive commit messages
- Keep functions focused and under 50 lines
- Document non-obvious logic with comments
- Update documentation when adding features

## Building and Testing
```bash
make debug  # Build with debug symbols
Xephyr :1 & sleep 1 && DISPLAY=:1 ./sWM
```

## Code of Conduct

Be respectful and inclusive. We follow the [https://www.contributor-covenant.org/](Contributor Covenant).
