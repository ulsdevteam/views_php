<?php
// $Id$

/**
 * A handler to provide an area that is constructed by the administrator using PHP.
 *
 * @ingroup views_area_handlers
 */
class views_php_handler_area extends views_handler_area {

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['php_output'] = array('default' => "<h4>Example PHP code</h4>\n<p>Time: <?php print date('H:i', time());?></p>\n");
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['php_output'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-setup'),
      '#title' => t('Output code'),
      '#default_value' => $this->options['php_output'],
      '#rows' => 5,
      '#description' => t('Code to construct the output of this area.') . ' <strong>' . t('Use &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong>',
    );
    views_php_check_input_access($form['php_output']);
    $form['php_output_variables'] = views_php_available_variables($form['php_output'], $this->view, $this, array('$view', '$handler', '$results'));
  }


  /**
   * Implements views_handler_area#render().
   */
  function render($empty = FALSE) {
    // Ecexute output PHP code.
    if ((!$empty || !empty($this->options['empty'])) && !empty($this->options['php_output'])) {
      $lamda_function = create_function('$view, $handler, $results', ' ?> ' . $this->options['php_output'] . ' <?php ');
      ob_start();
      $lamda_function($this->view, $this, $this->view->result);
      return ob_get_clean();
    }
    return '';
  }
}
