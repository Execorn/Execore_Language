# Configuration file for the Sphinx documentation builder.
# Execore Frontier C++2026 Documentation

project = 'Execore'
copyright = '2026, Execore Contributors'
author = 'Execore Core Team'
release = '2.0.0'

extensions = [
    'myst_parser',
    'sphinxcontrib.mermaid',
]

source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

root_doc = 'index'

exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

html_theme = 'alabaster'
html_static_path = []

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "dollarmath",
    "amsmath",
]

mermaid_version = "10.9.0"
