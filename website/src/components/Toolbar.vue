<template>
    <nav>
        <!-- Horizontale Toolbar-->
        <v-card>
            <v-toolbar>
                <v-toolbar-title class="mx-7 text-center font-weight-bold display-1">
                    Kicktool
                    <v-icon class="ma-1">mdi-hammer-wrench</v-icon>
                </v-toolbar-title>

                <v-spacer></v-spacer>

                <!-- darkmode -->
                <v-btn class="mx-4" @click="darkmode">
                    <v-icon>mdi-brightness-6</v-icon>
                </v-btn>

                <!-- Alle tabs-->
                <template v-slot:extension>
                    <v-tabs grow align-with-title>
                        <v-tabs-slider dark></v-tabs-slider>
                        <v-tab v-for="link in links" :key="link.text" router :to="link.route" class="text-center font-weight-bold">
                            {{ link.text }}
                            <v-icon class="ma-10" x-large>{{ link.icon }}</v-icon>
                        </v-tab>
                    </v-tabs>
                </template>

                <!-- Dropdown Menü Rechts Oben -->
                <v-menu bottom open-on-hover :close-on-click="closeOnClick" offset-y>
                    <template v-slot:activator="{ on, attrs }">
                        <v-btn v-bind="attrs" v-on="on">
                            <v-icon>mdi-menu-down</v-icon>
                        </v-btn>
                    </template>

                    <!-- Dropdown Menü für die tabs-->
                    <v-list>
                        <v-list-item v-for="link in links" :key="link.text" router :to="link.route">
                            <v-list-item-title class="text-center font-weight-bold">{{ link.text }}</v-list-item-title>
                        </v-list-item>
                    </v-list>
                </v-menu>
            </v-toolbar>
        </v-card>
        <!-- Ende Horizontale Toolbar-->
    </nav>
</template>

<script>
    export default {
        data() {
            return {
                drawer: false,
                closeOnClick: false,
                links: [
                    { icon: 'mdi-account-group', text: 'Session', route: '/session' },
                   // { icon: 'mdi-chart-timeline-variant-shimmer', text: 'History', route: '/history' },
                    { icon: 'mdi-account-cancel', text: 'MW2 Prison', route: '/prison' },
                ]
            };
        },
        methods: {
            darkmode() {
                this.$vuetify.theme.dark = !this.$vuetify.theme.dark
            }
        }
    }
</script>