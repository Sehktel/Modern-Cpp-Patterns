#!/usr/bin/env python3
"""
Скрипт автоматической синхронизации структуры курса.

Сканирует проект, проверяет наличие обязательных файлов для каждого паттерна
и генерирует актуальный COURSE_STRUCTURE_COMPLETE.md.

Использование:
    python sync_course_structure.py [--output FILE] [--check-only]

Опции:
    --output FILE       Путь к выходному файлу (по умолчанию: COURSE_STRUCTURE_COMPLETE.md)
    --check-only        Только проверить статус, не обновлять файл
    --verbose           Детальный вывод
    --json              Вывод в JSON формате
"""

import os
import sys
import json
import argparse
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Set, Tuple
from dataclasses import dataclass, field, asdict
from collections import defaultdict

# Корень проекта (относительно расположения скрипта)
PROJECT_ROOT = Path(__file__).parent.parent


@dataclass
class PatternStatus:
    """Статус реализации паттерна."""
    
    # Идентификация
    module_number: int
    module_name: str
    lesson_number: int
    lesson_name: str
    pattern_name: str
    directory: Path
    
    # Основные файлы
    has_readme: bool = False
    has_cmakelists: bool = False
    
    # Security файлы
    has_security_analysis: bool = False
    has_security_poster: bool = False
    has_vulnerabilities: bool = False
    has_exploits: bool = False
    has_secure_alternatives: bool = False
    
    # C++23 файлы
    has_cpp23_comparison: bool = False
    
    # Дополнительные файлы
    main_cpp_files: List[str] = field(default_factory=list)
    other_files: List[str] = field(default_factory=list)
    
    def completion_percentage(self) -> float:
        """Вычисляет процент завершённости паттерна."""
        # Веса компонентов
        weights = {
            'has_readme': 10,
            'has_cmakelists': 10,
            'has_security_analysis': 15,
            'has_security_poster': 15,
            'has_vulnerabilities': 15,
            'has_exploits': 10,
            'has_secure_alternatives': 15,
            'has_cpp23_comparison': 10,
        }
        
        total_weight = sum(weights.values())
        achieved_weight = sum(
            weight for attr, weight in weights.items() 
            if getattr(self, attr)
        )
        
        return (achieved_weight / total_weight) * 100
    
    def get_status_emoji(self) -> str:
        """Возвращает emoji статуса в зависимости от завершённости."""
        completion = self.completion_percentage()
        if completion >= 90:
            return "✅"
        elif completion >= 70:
            return "🟢"
        elif completion >= 50:
            return "🟡"
        elif completion >= 30:
            return "🟠"
        else:
            return "🔴"
    
    def get_status_text(self) -> str:
        """Возвращает текстовый статус."""
        completion = self.completion_percentage()
        if completion >= 90:
            return "ПОЛНОСТЬЮ РЕАЛИЗОВАН"
        elif completion >= 70:
            return "ПРАКТИЧЕСКИ ЗАВЕРШЁН"
        elif completion >= 50:
            return "ЧАСТИЧНО РЕАЛИЗОВАН"
        elif completion >= 30:
            return "БАЗОВАЯ СТРУКТУРА"
        else:
            return "МИНИМАЛЬНАЯ СТРУКТУРА"


@dataclass
class ModuleStats:
    """Статистика по модулю."""
    module_number: int
    module_name: str
    patterns: List[PatternStatus] = field(default_factory=list)
    
    def total_patterns(self) -> int:
        return len(self.patterns)
    
    def average_completion(self) -> float:
        if not self.patterns:
            return 0.0
        return sum(p.completion_percentage() for p in self.patterns) / len(self.patterns)


class CourseStructureScanner:
    """Сканер структуры курса."""
    
    # Определение модулей курса
    MODULES = {
        '01-basics': (1, 'Основы C++'),
        '02-principles': (2, 'Принципы проектирования'),
        '03-creational': (3, 'Порождающие паттерны'),
        '04-structural': (4, 'Структурные паттерны'),
        '05-behavioral': (5, 'Поведенческие паттерны'),
        '06-modern-cpp': (6, 'Современный C++'),
        '07-concurrency': (7, 'Паттерны многопоточности'),
        '08-high-load': (8, 'Паттерны высоконагруженных систем'),
        '09-performance': (9, 'Паттерны производительности'),
    }
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.patterns: List[PatternStatus] = []
        self.modules: Dict[int, ModuleStats] = {}
    
    def scan(self) -> None:
        """Сканирует всю структуру проекта."""
        print(f"🔍 Сканирование проекта: {self.project_root}")
        
        for module_dir, (module_num, module_name) in self.MODULES.items():
            module_path = self.project_root / module_dir
            
            if not module_path.exists():
                print(f"⚠️  Модуль не найден: {module_dir}")
                continue
            
            print(f"📂 Сканирование модуля {module_num}: {module_name}")
            module_stats = ModuleStats(module_num, module_name)
            
            # Ищем все директории уроков (lesson_*)
            for lesson_dir in sorted(module_path.glob('lesson_*')):
                if not lesson_dir.is_dir():
                    continue
                
                pattern_status = self._scan_lesson(module_num, module_name, lesson_dir)
                if pattern_status:
                    self.patterns.append(pattern_status)
                    module_stats.patterns.append(pattern_status)
            
            if module_stats.patterns:
                self.modules[module_num] = module_stats
        
        print(f"✅ Сканирование завершено. Найдено паттернов: {len(self.patterns)}")
    
    def _scan_lesson(self, module_num: int, module_name: str, lesson_dir: Path) -> PatternStatus:
        """Сканирует директорию урока."""
        # Парсим имя директории: lesson_X_Y_pattern_name
        parts = lesson_dir.name.split('_')
        if len(parts) < 3:
            return None
        
        lesson_num = int(parts[1])
        pattern_name = '_'.join(parts[3:]) if len(parts) > 3 else parts[2]
        lesson_name = pattern_name.replace('_', ' ').title()
        
        status = PatternStatus(
            module_number=module_num,
            module_name=module_name,
            lesson_number=lesson_num,
            lesson_name=lesson_name,
            pattern_name=pattern_name,
            directory=lesson_dir.relative_to(self.project_root)
        )
        
        # Проверяем наличие основных файлов
        status.has_readme = (lesson_dir / 'README.md').exists()
        status.has_cmakelists = (lesson_dir / 'CMakeLists.txt').exists()
        
        # Проверяем security файлы
        status.has_security_analysis = (lesson_dir / 'SECURITY_ANALYSIS.md').exists()
        status.has_vulnerabilities = any(lesson_dir.glob('*_vulnerabilities.cpp'))
        status.has_secure_alternatives = any(lesson_dir.glob('secure_*_alternatives.cpp'))
        
        # Проверяем exploits директорию
        exploits_dir = lesson_dir / 'exploits'
        status.has_exploits = exploits_dir.exists() and any(exploits_dir.glob('*.cpp'))
        
        # Проверяем security poster (может быть в разных форматах)
        poster_patterns = [
            f'*SECURITY_POSTER.md',
            f'{pattern_name.upper()}_SECURITY_POSTER.md',
        ]
        status.has_security_poster = any(
            lesson_dir.glob(pattern) for pattern in poster_patterns
        )
        
        # Проверяем C++23 comparison файлы
        cpp23_patterns = [
            f'*_cpp23_comparison.cpp',
            f'*_cpp23_*.cpp',
        ]
        status.has_cpp23_comparison = any(
            lesson_dir.glob(pattern) for pattern in cpp23_patterns
        )
        
        # Собираем список .cpp файлов
        for cpp_file in lesson_dir.glob('*.cpp'):
            if 'vulnerabilities' not in cpp_file.name and \
               'secure_' not in cpp_file.name and \
               'cpp23' not in cpp_file.name:
                status.main_cpp_files.append(cpp_file.name)
        
        # Собираем другие файлы
        for md_file in lesson_dir.glob('*.md'):
            if md_file.name not in ['README.md', 'SECURITY_ANALYSIS.md'] and \
               'SECURITY_POSTER' not in md_file.name:
                status.other_files.append(md_file.name)
        
        return status
    
    def get_overall_stats(self) -> Dict:
        """Возвращает общую статистику."""
        total_patterns = len(self.patterns)
        avg_completion = sum(p.completion_percentage() for p in self.patterns) / total_patterns if total_patterns else 0
        
        # Подсчёт файлов
        total_files = {
            'readme': sum(1 for p in self.patterns if p.has_readme),
            'cmakelists': sum(1 for p in self.patterns if p.has_cmakelists),
            'security_analysis': sum(1 for p in self.patterns if p.has_security_analysis),
            'security_poster': sum(1 for p in self.patterns if p.has_security_poster),
            'vulnerabilities': sum(1 for p in self.patterns if p.has_vulnerabilities),
            'exploits': sum(1 for p in self.patterns if p.has_exploits),
            'secure_alternatives': sum(1 for p in self.patterns if p.has_secure_alternatives),
            'cpp23_comparison': sum(1 for p in self.patterns if p.has_cpp23_comparison),
        }
        
        return {
            'total_patterns': total_patterns,
            'average_completion': avg_completion,
            'files': total_files,
            'modules': len(self.modules),
        }
    
    def generate_markdown(self) -> str:
        """Генерирует markdown отчёт."""
        lines = []
        
        # Заголовок
        lines.append("# 📁 Полная структура курса - Автоматически сгенерировано")
        lines.append("")
        lines.append(f"**Дата генерации**: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        lines.append(f"**Генератор**: `tools/sync_course_structure.py`")
        lines.append("")
        lines.append("---")
        lines.append("")
        
        # Общая статистика
        stats = self.get_overall_stats()
        lines.append("## 📊 Общая статистика")
        lines.append("")
        lines.append(f"**Всего паттернов**: {stats['total_patterns']}")
        lines.append(f"**Средняя завершённость**: {stats['average_completion']:.1f}%")
        lines.append(f"**Модулей**: {stats['modules']}")
        lines.append("")
        
        # Таблица покрытия
        lines.append("### Покрытие компонентов")
        lines.append("")
        lines.append("| Компонент | Реализовано | Процент |")
        lines.append("|-----------|-------------|---------|")
        
        for component, count in stats['files'].items():
            percentage = (count / stats['total_patterns']) * 100
            emoji = "✅" if percentage >= 90 else "🟡" if percentage >= 70 else "🔴"
            component_name = component.replace('_', ' ').title()
            lines.append(f"| {component_name} | {count}/{stats['total_patterns']} | {emoji} {percentage:.0f}% |")
        
        lines.append("")
        lines.append("---")
        lines.append("")
        
        # Детали по модулям
        lines.append("## 📚 Детали по модулям")
        lines.append("")
        
        for module_num in sorted(self.modules.keys()):
            module = self.modules[module_num]
            avg_comp = module.average_completion()
            
            lines.append(f"### Модуль {module_num}: {module.module_name}")
            lines.append("")
            lines.append(f"**Паттернов**: {module.total_patterns()} | **Завершённость**: {avg_comp:.1f}%")
            lines.append("")
            
            # Таблица паттернов модуля
            lines.append("| # | Паттерн | Статус | Завершено | Компоненты |")
            lines.append("|---|---------|--------|-----------|------------|")
            
            for pattern in module.patterns:
                components = []
                if pattern.has_readme: components.append("📖")
                if pattern.has_security_analysis: components.append("🔒")
                if pattern.has_security_poster: components.append("📋")
                if pattern.has_vulnerabilities: components.append("⚠️")
                if pattern.has_exploits: components.append("💣")
                if pattern.has_secure_alternatives: components.append("✅")
                if pattern.has_cpp23_comparison: components.append("🚀")
                
                components_str = " ".join(components) if components else "—"
                
                lines.append(
                    f"| {pattern.lesson_number} | {pattern.lesson_name} | "
                    f"{pattern.get_status_emoji()} {pattern.get_status_text()} | "
                    f"{pattern.completion_percentage():.0f}% | {components_str} |"
                )
            
            lines.append("")
            
            # Детали по каждому паттерну
            for pattern in module.patterns:
                self._add_pattern_details(lines, pattern)
            
            lines.append("---")
            lines.append("")
        
        # Легенда
        lines.append("## 📖 Легенда")
        lines.append("")
        lines.append("### Компоненты")
        lines.append("- 📖 README.md - документация урока")
        lines.append("- 🔒 SECURITY_ANALYSIS.md - анализ безопасности")
        lines.append("- 📋 *_SECURITY_POSTER.md - security плакат")
        lines.append("- ⚠️ *_vulnerabilities.cpp - уязвимые реализации")
        lines.append("- 💣 exploits/*_exploits.cpp - эксплоиты")
        lines.append("- ✅ secure_*_alternatives.cpp - безопасные альтернативы")
        lines.append("- 🚀 *_cpp23_comparison.cpp - C++23 сравнение")
        lines.append("")
        
        lines.append("### Статусы")
        lines.append("- ✅ ПОЛНОСТЬЮ РЕАЛИЗОВАН (≥90%)")
        lines.append("- 🟢 ПРАКТИЧЕСКИ ЗАВЕРШЁН (70-89%)")
        lines.append("- 🟡 ЧАСТИЧНО РЕАЛИЗОВАН (50-69%)")
        lines.append("- 🟠 БАЗОВАЯ СТРУКТУРА (30-49%)")
        lines.append("- 🔴 МИНИМАЛЬНАЯ СТРУКТУРА (<30%)")
        lines.append("")
        
        lines.append("---")
        lines.append("")
        lines.append(f"*Автоматически сгенерировано: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}*")
        
        return "\n".join(lines)
    
    def _add_pattern_details(self, lines: List[str], pattern: PatternStatus) -> None:
        """Добавляет детали паттерна в отчёт."""
        lines.append(f"#### Урок {pattern.module_number}.{pattern.lesson_number}: {pattern.lesson_name} "
                    f"{pattern.get_status_emoji()} ({pattern.completion_percentage():.0f}%)")
        lines.append("")
        
        # Файлы паттерна
        lines.append("**Структура:**")
        
        # Основные файлы
        lines.append(f"- README.md {'✅' if pattern.has_readme else '❌'}")
        lines.append(f"- CMakeLists.txt {'✅' if pattern.has_cmakelists else '❌'}")
        
        # Main cpp files
        if pattern.main_cpp_files:
            for cpp_file in sorted(pattern.main_cpp_files):
                lines.append(f"- {cpp_file} ✅")
        
        # Security components
        lines.append("")
        lines.append("**Security:**")
        lines.append(f"- SECURITY_ANALYSIS.md {'✅' if pattern.has_security_analysis else '❌'}")
        lines.append(f"- Security Poster {'✅' if pattern.has_security_poster else '❌'}")
        lines.append(f"- Vulnerabilities {'✅' if pattern.has_vulnerabilities else '❌'}")
        lines.append(f"- Exploits {'✅' if pattern.has_exploits else '❌'}")
        lines.append(f"- Secure Alternatives {'✅' if pattern.has_secure_alternatives else '❌'}")
        
        # C++23
        lines.append("")
        lines.append("**Modern C++:**")
        lines.append(f"- C++23 Comparison {'✅' if pattern.has_cpp23_comparison else '❌'}")
        
        # Дополнительные файлы
        if pattern.other_files:
            lines.append("")
            lines.append("**Дополнительно:**")
            for other_file in sorted(pattern.other_files):
                lines.append(f"- {other_file} ✅")
        
        lines.append("")


def main():
    """Главная функция."""
    parser = argparse.ArgumentParser(
        description='Автоматическая синхронизация структуры курса',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument(
        '--output', '-o',
        type=Path,
        default=PROJECT_ROOT / 'COURSE_STRUCTURE_COMPLETE.md',
        help='Путь к выходному файлу (по умолчанию: COURSE_STRUCTURE_COMPLETE.md)'
    )
    
    parser.add_argument(
        '--check-only', '-c',
        action='store_true',
        help='Только проверить статус, не обновлять файл'
    )
    
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Детальный вывод'
    )
    
    parser.add_argument(
        '--json', '-j',
        action='store_true',
        help='Вывод в JSON формате'
    )
    
    args = parser.parse_args()
    
    # Создаём сканер и запускаем сканирование
    scanner = CourseStructureScanner(PROJECT_ROOT)
    scanner.scan()
    
    # JSON вывод
    if args.json:
        data = {
            'timestamp': datetime.now().isoformat(),
            'stats': scanner.get_overall_stats(),
            'patterns': [asdict(p) for p in scanner.patterns],
        }
        print(json.dumps(data, indent=2, default=str))
        return 0
    
    # Статистика
    stats = scanner.get_overall_stats()
    print("\n" + "="*60)
    print("📊 ОБЩАЯ СТАТИСТИКА")
    print("="*60)
    print(f"Всего паттернов: {stats['total_patterns']}")
    print(f"Средняя завершённость: {stats['average_completion']:.1f}%")
    print(f"Модулей: {stats['modules']}")
    print()
    
    print("Покрытие компонентов:")
    for component, count in sorted(stats['files'].items()):
        percentage = (count / stats['total_patterns']) * 100
        emoji = "✅" if percentage >= 90 else "🟡" if percentage >= 70 else "🔴"
        component_name = component.replace('_', ' ').title()
        print(f"  {emoji} {component_name:25} {count:2}/{stats['total_patterns']:2} ({percentage:5.1f}%)")
    
    # Генерируем и сохраняем markdown
    if not args.check_only:
        print(f"\n📝 Генерация отчёта...")
        markdown = scanner.generate_markdown()
        
        # Сохраняем файл
        output_path = args.output
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(markdown, encoding='utf-8')
        
        print(f"✅ Отчёт сохранён: {output_path}")
        print(f"   Размер: {len(markdown)} символов")
    else:
        print("\n⚠️  Режим --check-only: файл не обновлён")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())

