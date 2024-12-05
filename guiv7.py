import dash
from dash import dcc, html, Input, Output, State
import plotly.graph_objects as go
import math

# Nodo de Octree
class Node:
    def __init__(self, center, half_size):
        self.center = center
        self.half_size = half_size
        self.is_leaf = True
        self.points = []
        self.children = [None] * 8

    def find_child_to_insert(self, point):
        x, y, z = point
        cx, cy, cz = self.center
        index = 0
        index |= (x >= cx) << 0
        index |= (y >= cy) << 1
        index |= (z >= cz) << 2
        return index

    def split(self):
        self.is_leaf = False
        quarter = self.half_size / 2

        for i in range(8):
            offsetX = quarter if (i & 1) else -quarter
            offsetY = quarter if (i & 2) else -quarter
            offsetZ = quarter if (i & 4) else -quarter

            new_center = (
                self.center[0] + offsetX,
                self.center[1] + offsetY,
                self.center[2] + offsetZ
            )
            self.children[i] = Node(new_center, quarter)

        for point in self.points:
            child_index = self.find_child_to_insert(point)
            self.children[child_index].insert(point)

        self.points = []

    def insert(self, point):
        if self.is_leaf:
            self.points.append(point)
            if len(self.points) > 2:
                self.split()
        else:
            child_index = self.find_child_to_insert(point)
            self.children[child_index].insert(point)

    def remove(self, point):
        if self.is_leaf:
            if point in self.points:
                self.points.remove(point)
                return True
            return False
        else:
            child_index = self.find_child_to_insert(point)
            if self.children[child_index] and self.children[child_index].remove(point):
                # Verifica si todos los hijos están vacíos
                if all(child is None or (child.is_leaf and not child.points) for child in self.children):
                    self.points = []
                    for child in self.children:
                        if child:
                            self.points.extend(child.points)
                    self.children = [None] * 8
                    self.is_leaf = True
                return True
        return False

    def radius_query(self, query, radius):
        result = []

        def search_in_radius(node):
            if not node:
                return

            distance_to_node = math.sqrt(sum((c1 - c2) ** 2 for c1, c2 in zip(node.center, query)))
            if distance_to_node > node.half_size + radius:
                return

            if node.is_leaf:
                for point in node.points:
                    distance = math.sqrt(sum((c1 - c2) ** 2 for c1, c2 in zip(point, query)))
                    if distance <= radius:
                        result.append(point)
            else:
                for child in node.children:
                    search_in_radius(child)

        search_in_radius(self)
        return result

    def _1nn_search(self, query):
        closest_point = None
        min_distance = float('inf')

        def search_nearest(node):
            nonlocal closest_point, min_distance
            if not node:
                return

            distance_to_node = math.sqrt(sum((c1 - c2) ** 2 for c1, c2 in zip(node.center, query)))
            if distance_to_node > node.half_size + min_distance:
                return

            if node.is_leaf:
                for point in node.points:
                    distance = math.sqrt(sum((c1 - c2) ** 2 for c1, c2 in zip(point, query)))
                    if distance < min_distance:
                        min_distance = distance
                        closest_point = point
            else:
                for child in node.children:
                    search_nearest(child)

        search_nearest(self)
        return closest_point

# Clase Octree
class Octree:
    def __init__(self, center, half_size):
        self.root = Node(center, half_size)

    def insert(self, point):
        self.root.insert(point)

    def remove(self, point):
        return self.root.remove(point)

    def radius_query(self, query, radius):
        return self.root.radius_query(query, radius)

    def _1nn_search(self, query):
        return self.root._1nn_search(query)

# Visualizador Interactivo
app = dash.Dash(__name__)
fig = go.Figure()

# Inicializa el Octree
octree = Octree(center=(5, 5, 5), half_size=5)

# Función para dibujar nodos
def draw_node(node, fig, colors=None, depth=0):
    if colors is None:
        colors = ['blue', 'green', 'red', 'yellow', 'purple', 'orange', 'cyan', 'magenta']
    if node.is_leaf:
        draw_cube(fig, node.center, node.half_size, color=colors[depth % len(colors)])
        for point in node.points:
            fig.add_trace(go.Scatter3d(
                x=[point[0]], y=[point[1]], z=[point[2]],
                mode='markers',
                marker=dict(size=5, color='red'),
                name="Point"
            ))
    else:
        for child in node.children:
            if child:
                draw_node(child, fig, colors, depth + 1)

def draw_cube(fig, center, half_size, color='blue', opacity=0.3):
    x, y, z = center
    hs = half_size
    vertices = [
        [x - hs, y - hs, z - hs], [x + hs, y - hs, z - hs], [x + hs, y + hs, z - hs], [x - hs, y + hs, z - hs],
        [x - hs, y - hs, z + hs], [x + hs, y - hs, z + hs], [x + hs, y + hs, z + hs], [x - hs, y + hs, z + hs]
    ]

    faces = [
        [vertices[0], vertices[1], vertices[2], vertices[3]],
        [vertices[4], vertices[5], vertices[6], vertices[7]],
        [vertices[0], vertices[1], vertices[5], vertices[4]],
        [vertices[2], vertices[3], vertices[7], vertices[6]],
        [vertices[1], vertices[2], vertices[6], vertices[5]],
        [vertices[4], vertices[7], vertices[3], vertices[0]],
    ]

    for face in faces:
        x, y, z = zip(*face)
        fig.add_trace(go.Mesh3d(
            x=x, y=y, z=z,
            color=color,
            opacity=opacity
        ))

# Layout de la app
app.layout = html.Div([
    html.H1("Octree Implementation"),
    dcc.Graph(id='octree-plot', figure=fig),
    html.Div([
        html.Label("Point X, Y, Z:"),
        dcc.Input(id='x-input', type='number', value=5),
        dcc.Input(id='y-input', type='number', value=5),
        dcc.Input(id='z-input', type='number', value=5),
        html.Button("Add Point", id='add-button', n_clicks=0),
        html.Button("Remove Point", id='remove-button', n_clicks=0, style={'margin-left': '10px'}),
        html.Button("Reset Octree", id='reset-button', n_clicks=0, style={'margin-left': '10px'}),
    ]),
    html.Div([
        html.Label("Query Point X, Y, Z:"),
        dcc.Input(id='query-x', type='number', value=5),
        dcc.Input(id='query-y', type='number', value=5),
        dcc.Input(id='query-z', type='number', value=5),
        html.Label("Radius (for Radius Query):"),
        dcc.Input(id='radius-input', type='number', value=2),
        html.Button("Radius Query", id='radius-query-btn', n_clicks=0, style={'margin-left': '10px'}),
        html.Button("1-NN Query", id='_1nn-query-btn', n_clicks=0, style={'margin-left': '10px'})
    ]),
])

# Callback para manejar todas las acciones
@app.callback(
    Output('octree-plot', 'figure'),
    [Input('add-button', 'n_clicks'),
     Input('remove-button', 'n_clicks'),
     Input('reset-button', 'n_clicks'),
     Input('radius-query-btn', 'n_clicks'),
     Input('_1nn-query-btn', 'n_clicks')],
    [State('x-input', 'value'),
     State('y-input', 'value'),
     State('z-input', 'value'),
     State('query-x', 'value'),
     State('query-y', 'value'),
     State('query-z', 'value'),
     State('radius-input', 'value')]
)
def handle_actions(add_clicks, remove_clicks, reset_clicks, radius_clicks, nn_clicks,
                   x, y, z, query_x, query_y, query_z, radius):
    global octree, fig
    ctx = dash.callback_context

    if ctx.triggered:
        action = ctx.triggered[0]['prop_id'].split('.')[0]

        if action == 'add-button' and add_clicks > 0:
            octree.insert((x, y, z))
        elif action == 'remove-button' and remove_clicks > 0:
            octree.remove((x, y, z))
        elif action == 'reset-button' and reset_clicks > 0:
            octree = Octree(center=(5, 5, 5), half_size=5)
        elif action == 'radius-query-btn' and radius_clicks > 0:
            query_point = (query_x, query_y, query_z)
            points_in_radius = octree.radius_query(query_point, radius)
            fig = go.Figure()
            draw_node(octree.root, fig)
            fig.add_trace(go.Scatter3d(
                x=[p[0] for p in points_in_radius],
                y=[p[1] for p in points_in_radius],
                z=[p[2] for p in points_in_radius],
                mode='markers',
                marker=dict(size=8, color='green'),
                name="Points in Radius"
            ))
        elif action == '_1nn-query-btn' and nn_clicks > 0:
            query_point = (query_x, query_y, query_z)
            nearest_point = octree._1nn_search(query_point)
            fig = go.Figure()
            draw_node(octree.root, fig)
            # Dibuja el vecino más cercano
            fig.add_trace(go.Scatter3d(
                x=[nearest_point[0]], y=[nearest_point[1]], z=[nearest_point[2]],
                mode='markers',
                marker=dict(size=10, color='orange'),
                name="Nearest Point"
            ))
            # Dibuja la flecha como una línea
            fig.add_trace(go.Scatter3d(
                x=[query_x, nearest_point[0]],
                y=[query_y, nearest_point[1]],
                z=[query_z, nearest_point[2]],
                mode='lines+markers',
                line=dict(color='orange', width=4),
                marker=dict(size=5, color='blue'),
                name="1-NN Line"
            ))

    fig = go.Figure()
    draw_node(octree.root, fig)
    return fig

# Ejecuta la aplicación
if __name__ == '__main__':
    app.run_server(debug=True)
