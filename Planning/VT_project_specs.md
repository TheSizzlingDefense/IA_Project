# Project Specifications: AI-Powered Vocabulary Flashcard Application

## Summary

This project involves developing a desktop application for vocabulary flashcard creation and review, specifically designed for language learners. The application will combine a C++ implementation of SuperMemo's spaced repetition algorithm with AI-powered features to significantly reduce the time required to create high-quality flashcards. Unlike web-based alternatives, this application will prioritize local data processing and storage while leveraging modern AI models to generate contextual examples, definitions, and mnemonic devices automatically from user-inputted vocabulary words.

## Vision

- **Efficiency in Learning Preparation**: Drastically reduce the time investment needed for flashcard creation through intelligent automation
- **Local-First Philosophy**: Maintain user privacy and data control by processing information locally whenever possible
- **Adaptive Learning Experience**: Create personalized learning paths using proven SuperMemo algorithms that respond to individual progress and retention patterns
- **Proven Learning Science**: Implement time-tested spaced repetition methodology with modern AI enhancements

## Product Requirements

### Core Functionality

#### Flashcard Management
- **Card Creation Interface**: Simple text input for target vocabulary words with automatic generation of related fields
- **Card Editing**: Manual override capability for all AI-generated content with intuitive editing controls
- **Deck Organization**: Hierarchical deck system with tagging, search, and filtering capabilities
- **Import/Export**: Support for common formats (CSV, JSON) with batch operation handling

#### Spaced Repetition System (SRS)
- **SuperMemo Algorithm**: Native C++ implementation of SuperMemo (SM-2 or later) for optimal performance and customization
- **Adaptive Scheduling**: Algorithm that adjusts review intervals based on user performance metrics and retention patterns
- **Performance Analytics**: Visual feedback on retention rates, progress trends, and problem areas with algorithm-specific metrics
- **Review Interface**: Clean, distraction-free review mode with SuperMemo-compatible self-assessment options (Again, Hard, Good, Easy)
- **Algorithm Transparency**: Display of calculated intervals and ease factors to help users understand the scheduling system

### AI Integration Features

#### Automated Content Generation
- **Definition Extraction**: Automatic retrieval of word definitions in target and native languages
- **Contextual Example Sentences**: Generation of natural, practical usage examples
- **Mnemonic Device Creation**: AI-assisted creation of memory aids tailored to the vocabulary
- **Pronunciation Guides**: Phonetic transcription and audio generation capabilities
- **Related Vocabulary**: Suggestions for semantically connected words and phrases

#### Smart Processing
- **Context Awareness**: Ability to process multi-word expressions and phrases intelligently
- **Language Detection**: Automatic identification of input language and appropriate processing
- **Error Correction**: Detection and suggestions for misspelled or incorrect vocabulary entries

### Technical Requirements

#### Architecture & Performance
- **Local-First Data Storage**: SQLite database for flashcard storage and user data with SuperMemo algorithm parameters
- **Offline Capability**: Full functionality without internet connection after initial setup
- **Efficient Resource Usage**: Memory footprint under 500MB during normal operation
- **Fast Startup**: Application launch under 3 seconds on average hardware
- **Algorithm Efficiency**: Optimized C++ SuperMemo implementation for rapid interval calculations

#### AI Model Integration
- **Local AI Processing**: Integration with local language models (200M-7B parameter range)
- **Optional Cloud Enhancement**: Ability to use cloud-based AI APIs for improved accuracy (user-configurable)
- **Model Management**: Efficient downloading, updating, and caching of AI models
- **Privacy Protection**: Clear indication when data is processed locally vs. cloud services

### User Experience

#### Interface Design
- **Cross-Platform Compatibility**: Native support for Windows, macOS, and Linux
- **Accessibility**: Support for screen readers, keyboard navigation, and high contrast modes
- **Responsive Layout**: Adaptive interface that scales well from small to large displays
- **Minimalist Design**: Clean, uncluttered interface focused on the learning workflow

#### Customization
- **SRS Customization**: Adjustable SuperMemo algorithm parameters for advanced users (easiness factors, interval modifiers, etc.)
- **Theme Support**: Light/dark mode and customizable color schemes
- **Keyboard Shortcuts**: Comprehensive shortcut system for power users, especially during review sessions
- **Notification System**: Configurable review reminders based on SuperMemo's due date calculations

## Technology Stack Recommendations

### Application Framework
**Qt Framework (C++)** - Optimal choice for:
- True cross-platform native applications
- Excellent performance characteristics for algorithm-intensive operations
- Mature ecosystem with extensive documentation
- Native look and feel on all platforms
- Strong internationalization support

### Spaced Repetition System
**Custom C++ SuperMemo Implementation** - Key components:
- SM-2 algorithm as baseline (proven, simple, effective)
- Extensible architecture for future algorithm versions
- Efficient data structures for interval calculations
- Statistical tracking for algorithm validation

### AI/ML Integration
**ONNX Runtime** with **Transformers.cpp** or similar:
- Efficient inference of language models on CPU/GPU
- Broad model format support
- Minimal dependencies
- Good performance on consumer hardware

### Data Storage
**SQLite** - Ideal for:
- Serverless, file-based database
- Excellent performance for local applications
- ACID compliance
- Minimal setup requirements
- Efficient storage of SuperMemo algorithm state (E-factors, intervals, repetition history)

### Additional Libraries
- **SQLiteCpp** for modern C++ database access
- **libcurl** for optional HTTP requests
- **CMake** for cross-platform building
- **Catch2** for unit testing, especially for algorithm correctness

## Efficiency Considerations

### Development Efficiency
- Use Qt Creator for rapid UI development and debugging
- Leverage existing C++ libraries to avoid reinventing core functionality
- Implement modular architecture to enable parallel development
- Create comprehensive test suite for SuperMemo algorithm implementation

### Runtime Efficiency
- Implement lazy loading for large flashcard collections
- Use background threads for AI processing to maintain UI responsiveness
- Cache frequently accessed data and AI model outputs
- Optimize database queries with proper indexing, especially for due card retrieval
- Highly optimized SuperMemo calculations with pre-computed intervals where possible

### User Workflow Efficiency
- One-click flashcard generation from minimal input
- Batch operations for mass card creation/editing
- Smart defaults with progressive disclosure of advanced features
- Comprehensive keyboard navigation to minimize mouse dependency
- Quick rating system during reviews that integrates seamlessly with SuperMemo algorithm
- Algorithm-aware deck statistics showing projected review loads and retention rates

### Algorithm-Specific Optimizations
- Efficient due card selection using database indexes on next review dates
- Incremental algorithm updates to avoid recalculating entire decks
- Memory-efficient storage of repetition history for long-term learning analytics
- Fast interval calculations using pre-computed ease factor tables where applicable